//------------------------------------------------------------------------------
// Micropython-Convert Demonstrates:
//
// * How to convert Arduino Sketches to Micropython
// * How to use AST matchers to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Ashutosh Pandey (ashutoshpandey123456@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <string>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Expr.h"

using namespace std;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");

//IfStatementHandler Class: All Rewriting For IF statements done here.

class IfStmtHandler : public MatchFinder::MatchCallback {
public:
  IfStmtHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    // The matched 'if' statement was bound to 'ifStmt'.
    if (const IfStmt *IfS = Result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")) {
      const Stmt *Then = IfS->getThen();
      Rewrite.InsertText(Then->getBeginLoc(), "#if part\n", true, true);

      if (const Stmt *Else = IfS->getElse()) {
        Rewrite.InsertText(Else->getBeginLoc(), "#else part\n", true, true);
      }
    }
  }

private:
  Rewriter &Rewrite;
};

//ForLoopHandler Class: All Rewriting For For Loop statements done here.

class IncrementForLoopHandler : public MatchFinder::MatchCallback {
public:
  IncrementForLoopHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    Rewrite.InsertText(IncVar->getBeginLoc(), "/* increment */", true, true);
    Rewrite.ReplaceText(IncVar->getBeginLoc(), "print");
  }

private:
  Rewriter &Rewrite;
};

//PinMode Class: All Rewriting For PinMode statements done here.

class pinModeVariableHandler : public MatchFinder::MatchCallback {
public:
   pinModeVariableHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::CallExpr* pm = Results.Nodes.getNodeAs<clang::CallExpr>("pinMode");
    Rewrite.ReplaceText(pm->getBeginLoc(), "machine.pin");
  }

private:
  Rewriter &Rewrite;
};

//Handler for Void Loop() Class: All Rewriting For void loop statements done here. Void loop() is rewritten as While True:

class loopExprHandler : public MatchFinder::MatchCallback {
public:
   loopExprHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::FunctionDecl* loop = Results.Nodes.getNodeAs<clang::FunctionDecl>("loopexpr");
    Rewrite.RemoveText(loop->getLocation()); 
    Rewrite.RemoveText(loop->getEndLoc());
    Rewrite.ReplaceText(loop->getBeginLoc(), "While True:");
    Rewrite.ReplaceText(loop->getLocation(), " ");
  }

private:
  Rewriter &Rewrite;
};

//Handler for delay() function: delay() is rewritten as time.sleep_ms

class delayHandler : public MatchFinder::MatchCallback {
public:
   delayHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* delayfinder = Results.Nodes.getNodeAs<clang::Stmt>("delay");
    Rewrite.ReplaceText(delayfinder->getBeginLoc(), "time.sleep_ms");
  }

private:
  Rewriter &Rewrite;
};

//Handler for Void Setup() Class: Void Setup is Deleted as It does not occur in Micropython Statements

class setupHandler : public MatchFinder::MatchCallback {
public:
   setupHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::FunctionDecl* setupfinder = Results.Nodes.getNodeAs<clang::FunctionDecl>("setupfunc"); 
    Rewrite.RemoveText(setupfinder->getLocation());
    Rewrite.RemoveText(setupfinder->getBeginLoc());
    Rewrite.ReplaceText(setupfinder->getBeginLoc(), " ");
  }

private:
  Rewriter &Rewrite;
};

//Handler for CompoundStatements: Curly Braces are not required in Micropython and Can be removed

class compoundStmtHandler : public MatchFinder::MatchCallback {
public:
   compoundStmtHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* compoundstmtfinder = Results.Nodes.getNodeAs<clang::Stmt>("compoundstmt");
    Rewrite.InsertText(compoundstmtfinder->getBeginLoc(), "#", true, true);
    Rewrite.InsertText(compoundstmtfinder->getEndLoc(), "#", true, true);
  }

private:
  Rewriter &Rewrite;
};


// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : HandlerForIf(R), HandlerForFor(R), HandlerForpinMode(R), HandlerForLoopExpr(R), HandlerForDelay(R), HandlerForSetup(R), HandlerForCompoundStmt(R){
    // Add a simple matcher for finding 'if' statements.
    Matcher.addMatcher(ifStmt().bind("ifStmt"), &HandlerForIf);

    // Add a complex matcher for finding 'for' loops with an initializer set
    // to 0, < comparison in the codition and an increment. For example:
    //
    //  for (int i = 0; i < N; ++i)
    Matcher.addMatcher(
        forStmt(hasLoopInit(declStmt(hasSingleDecl(
                    varDecl(hasInitializer(integerLiteral(equals(0))))
                        .bind("initVarName")))),
                hasIncrement(unaryOperator(
                    hasOperatorName("++"),
                    hasUnaryOperand(declRefExpr(to(
                        varDecl(hasType(isInteger())).bind("incVarName")))))),
                hasCondition(binaryOperator(
                    hasOperatorName("<"),
                    hasLHS(ignoringParenImpCasts(declRefExpr(to(
                        varDecl(hasType(isInteger())).bind("condVarName"))))),
                    hasRHS(expr(hasType(isInteger()))))))
            .bind("forLoop"),
        &HandlerForFor);

//Add A matcher for PinMode

Matcher.addMatcher(
	callExpr(isExpansionInMainFile(), hasAncestor(functionDecl(hasName("setup")))).bind("pinMode"), &HandlerForpinMode);

//Add A matcher for void_loop function of Arduino

Matcher.addMatcher(
  functionDecl(isExpansionInMainFile(), hasName("loop"), parameterCountIs(0)).bind("loopexpr"), &HandlerForLoopExpr); 

 //Add A matcher for delay() function
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), callExpr(hasDescendant(integerLiteral()))).bind("delay"), &HandlerForDelay);

 //Add A matcher to delete Void Setup() 
 Matcher.addMatcher(
   functionDecl(isExpansionInMainFile(), hasName("setup")).bind("setupfunc"), &HandlerForSetup); 

//Add A matcher to remove { } braces
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), compoundStmt()).bind("compoundstmt"), &HandlerForCompoundStmt);
}
  void HandleTranslationUnit(ASTContext &Context) override {
    // Run the matchers when we have the whole TU parsed.
    Matcher.matchAST(Context);
  }

private:
  IfStmtHandler HandlerForIf;
  IncrementForLoopHandler HandlerForFor;
  pinModeVariableHandler HandlerForpinMode;
  loopExprHandler HandlerForLoopExpr;
  delayHandler HandlerForDelay;
  setupHandler HandlerForSetup;
  compoundStmtHandler HandlerForCompoundStmt;
 
  MatchFinder Matcher;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
   SourceManager &SM = TheRewriter.getSourceMgr();
   llvm::errs() << "** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
//Now emit the Rewritten Buffer
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());

std::error_code error_code;
        llvm::raw_fd_ostream outFile("output.txt", error_code, llvm::sys::fs::F_None);
     TheRewriter.getEditBuffer(SM.getMainFileID()).write(outFile); // --> this will write the result>
    outFile.close();

  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
