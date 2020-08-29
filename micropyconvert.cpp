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
    const clang::CallExpr* delayfinder = Results.Nodes.getNodeAs<clang::CallExpr>("delay");
    Rewrite.ReplaceText(delayfinder->getBeginLoc(), "utime.sleep_ms");
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


//Handler for CompoundStatements: Curly Braces are not required in Micropython and Can be removed. Since  project with improper indentation might become hard to understand
// it will insert a # (comment statement) before each {}

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

//Handler for power expression. converts pow to math.pow

class powerHandler : public MatchFinder::MatchCallback {
public:
   powerHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* powfinder = Results.Nodes.getNodeAs<clang::Stmt>("pow");
    Rewrite.InsertText(powfinder->getBeginLoc(), "math.", true, true);
  }

private:
  Rewriter &Rewrite;
};

//Handler for square root expression. converts sqrt to math.sqrt

class sqrtHandler : public MatchFinder::MatchCallback {
public:
   sqrtHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* sqrtfinder = Results.Nodes.getNodeAs<clang::Stmt>("sqrt");
    Rewrite.InsertText(sqrtfinder->getBeginLoc(), "math.", true, true);
  }

private:
  Rewriter &Rewrite;
};

//Handler for sin expression. converts sin to math.sin

class sinHandler : public MatchFinder::MatchCallback {
public:
   sinHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* sinfinder = Results.Nodes.getNodeAs<clang::Stmt>("sin");
    Rewrite.InsertText(sinfinder->getBeginLoc(), "math.", true, true);
  }

private:
  Rewriter &Rewrite;
};

//Handler for cos expression. converts cos to math.cos

class cosHandler : public MatchFinder::MatchCallback {
public:
   cosHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* cosfinder = Results.Nodes.getNodeAs<clang::Stmt>("cos");
    Rewrite.InsertText(cosfinder->getBeginLoc(), "math.", true, true);
  }

private:
  Rewriter &Rewrite;
};

//Handler for tan expression. converts pow to math.tan

class tanHandler : public MatchFinder::MatchCallback {
public:
   tanHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::Stmt* tanfinder = Results.Nodes.getNodeAs<clang::Stmt>("tan");
    Rewrite.InsertText(tanfinder->getBeginLoc(), "math.", true, true);
  }

private:
  Rewriter &Rewrite;
};

//Handler for delay() function: delay() is rewritten as time.sleep_ms

class delayMicrosecondsHandler : public MatchFinder::MatchCallback {
public:
   delayMicrosecondsHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::CallExpr* delayMicrosecondsfinder = Results.Nodes.getNodeAs<clang::CallExpr>("delayMicroseconds");
    Rewrite.ReplaceText(delayMicrosecondsfinder->getBeginLoc(), "utime.sleep_us");
  }

private:
  Rewriter &Rewrite;
};

//Handler for delay() function: delay() is rewritten as time.sleep_ms

class millisHandler : public MatchFinder::MatchCallback {
public:
   millisHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::CallExpr* millisfinder = Results.Nodes.getNodeAs<clang::CallExpr>("millis");
    Rewrite.ReplaceText(millisfinder->getBeginLoc(), "utime.ticks_ms");
  }

private:
  Rewriter &Rewrite;
};

//Handler for delay() function: delay() is rewritten as time.sleep_ms

class microsHandler : public MatchFinder::MatchCallback {
public:
   microsHandler(Rewriter &Rewrite) : Rewrite(Rewrite)  {}

virtual void run(const MatchFinder::MatchResult &Results) {
    const clang::CallExpr* microsfinder = Results.Nodes.getNodeAs<clang::CallExpr>("micros");
    Rewrite.ReplaceText(microsfinder->getBeginLoc(), "utime.ticks_us");
  }

private:
  Rewriter &Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : HandlerForIf(R), HandlerForFor(R), HandlerForpinMode(R), HandlerForLoopExpr(R), HandlerForDelay(R), HandlerForSetup(R), HandlerForCompoundStmt(R), 
  HandlerForPower(R), HandlerForSqrt(R), HandlerForSin(R), HandlerForCos(R), HandlerForTan(R), HandlerForDelayMicroseconds(R), HandlerForMillis(R), HandlerForMicros(R){
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
	callExpr(isExpansionInMainFile(), callee(functionDecl(hasName("pinMode")))).bind("pinMode"), &HandlerForpinMode);

//Add A matcher for void_loop function of Arduino

Matcher.addMatcher(
  functionDecl(isExpansionInMainFile(), hasName("loop"), parameterCountIs(0)).bind("loopexpr"), &HandlerForLoopExpr); 

 //Add A matcher for delay() function
Matcher.addMatcher(
  callExpr(isExpansionInMainFile(), callee(functionDecl(hasName("delay")))).bind("delay"), &HandlerForDelay);

 //Add A matcher to delete Void Setup() 
 Matcher.addMatcher(
   functionDecl(isExpansionInMainFile(), hasName("setup")).bind("setupfunc"), &HandlerForSetup); 

//Add A matcher to remove { } braces
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), compoundStmt()).bind("compoundstmt"), &HandlerForCompoundStmt);

//Add a matcher to convert power to math.pow
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), has(declRefExpr(throughUsingDecl(hasName("pow"))))).bind("pow"), &HandlerForPower);

//Add a matcher to convert sqrt to math.sqrt
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), has(declRefExpr(throughUsingDecl(hasName("sqrt"))))).bind("sqrt"), &HandlerForSqrt);

//Add a matcher to convert sin to math.sin
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), has(declRefExpr(throughUsingDecl(hasName("sin"))))).bind("sin"), &HandlerForSin);

//Add a matcher to convert cos to math.cos
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), has(declRefExpr(throughUsingDecl(hasName("cos"))))).bind("cos"), &HandlerForCos);

//Add a matcher to convert tan to math.tan
Matcher.addMatcher(
  stmt(isExpansionInMainFile(), has(declRefExpr(throughUsingDecl(hasName("tan"))))).bind("tan"), &HandlerForTan);

//Add a matcher to convert delayMicroseconds() to its Micropython equivalent.
Matcher.addMatcher(
  callExpr(isExpansionInMainFile(), callee(functionDecl(hasName("delayMicroseconds")))).bind("delayMicroseconds"), &HandlerForDelayMicroseconds);

//Add a matcher to convert millis() to its Micropython equivalent.
Matcher.addMatcher(
  callExpr(isExpansionInMainFile(), callee(functionDecl(hasName("millis")))).bind("millis"), &HandlerForMillis);

//Add a matcher to convert micros() to its Micropython equivalent.
Matcher.addMatcher(
  callExpr(isExpansionInMainFile(), callee(functionDecl(hasName("micros")))).bind("micros"), &HandlerForMicros);
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
  powerHandler HandlerForPower;
  sqrtHandler HandlerForSqrt;
  sinHandler HandlerForSin;
  cosHandler HandlerForCos;
  tanHandler HandlerForTan;
  delayMicrosecondsHandler HandlerForDelayMicroseconds;
  millisHandler HandlerForMillis;
  microsHandler HandlerForMicros;

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
