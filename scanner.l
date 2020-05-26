%{
    #include "stdio.h"
    #include "scanner.h"
%}

%%


"/*"((("*"[^/])?)|[^*])*"*/" return COMMENT;


"//".*					return COMMENT;

^"#".*					return PREPROCESSOR;

digitalRead               return KEYWORD;
digitalWrite              return KEYWORD;
pinMODE                   return KEYWORD;
analogRead                return KEYWORD;
analogReference           return KEYWORD;
analogWrite               return KEYWORD;
analogReadResolution      return KEYWORD;
analogWriteResolution     return KEYWORD;
noTone                    return KEYWORD;
pulseIn                   return KEYWORD;
pulseInLong               return KEYWORD;
shiftIn                   return KEYWORD;
shiftOut                  return KEYWORD;
tone                      return KEYWORD;
delay                     return KEYWORD;
delayMicroseconds         return KEYWORD;
micros                    return KEYWORD;
millis                    return KEYWORD;
IsAlpha                   return KEYWORD;
isAlphaNumeric            return KEYWORD;
isAscii                   return KEYWORD;
isControl                 return KEYWORD;
isDigit                   return KEYWORD;
isGraph                   return KEYWORD;
isHexadecimalDigit        return KEYWORD;
isLowerCase               return KEYWORD;
isPrintable               return KEYWORD;
isPunct                   return KEYWORD;
isSpace                   return KEYWORD;
isUpperCase               return KEYWORD;
isWhitespace              return KEYWORD;
random                    return KEYWORD;
randomSeed                return KEYWORD;
bit                       return KEYWORD;
bitClear                  return KEYWORD;
bitRead                   return KEYWORD;
bitSet                    return KEYWORD;
bitWrite                  return KEYWORD;
highByte                  return KEYWORD;
lowByte                   return KEYWORD;
attachInterrupt           return KEYWORD;
detachInterrupt           return KEYWORD;
Interrupts                return KEYWORD;
noInterrupts              return KEYWORD;
if                        return KEYWORD;
Serial                    return KEYWORD;
available                 return KEYWORD;
availableForWrite         return KEYWORD;
begin                     return KEYWORD;
end                       return KEYWORD;
find                      return KEYWORD;
findUntil                 return KEYWORD;
flush                     return KEYWORD;
parseFloat                return KEYWORD;
parseInt                  return KEYWORD;
peek                      return KEYWORD;
print                     return KEYWORD;
println                   return KEYWORD;
read                      return KEYWORD;
readBytes                 return KEYWORD;
readBytesUntil            return KEYWORD;
readStringUntil           return KEYWORD;
setTimeout                return KEYWORD;
write                     return KEYWORD;
serialEvent               return KEYWORD;
requestFrom               return KEYWORD;
beginTransmission         return KEYWORD;
endTransmission           return KEYWORD;
SetClock                  return KEYWORD;
onReceive                 return KEYWORD;
onRequest                 return KEYWORD;
dnsServerIP               return KEYWORD;
gatewayIP                 return KEYWORD;
hardwareStatus            return KEYWORD;
init                      return KEYWORD;
linkStatus                return KEYWORD;
localIP                   return KEYWORD;
MACAddress                return KEYWORD;
maintain                  return KEYWORD;
setDnsServerIP            return KEYWORD;
setGatewayIP              return KEYWORD;
setLocalIP                return KEYWORD;
setMACAddress             return KEYWORD;
setRetransmissionCount    return KEYWORD;
setRetransmissionTimeout  return KEYWORD;
setSubnetMask             return KEYWORD;
subnetMask                return KEYWORD;
IPAddress                 return KEYWORD;
Server                    return KEYWORD;
EthernetServer            return KEYWORD;
accept                    return KEYWORD;
Client                    return KEYWORD;
EthernetClient            return KEYWORD;
connected                 return KEYWORD;
connect                   return KEYWORD;
localPort                 return KEYWORD;
setConnectionTimeout      return KEYWORD;
stop                      return KEYWORD;
beginPacket               return KEYWORD;
endPacket                 return KEYWORD;
parsePacket               return KEYWORD;
remoteIP                  return KEYWORD;
remotePort                return KEYWORD;
exists                    return KEYWORD;
mkdir                     return KEYWORD;
open                      return KEYWORD;
remove                    return KEYWORD;
rmdir                     return KEYWORD;
seek                      return KEYWORD;
size                      return KEYWORD;
IsDirectory               return KEYWORD;
openNextFile              return KEYWORD;
rewindDirectory           return KEYWORD;
array                     return KEYWORD;
bool                      return KEYWORD;
boolean                   return KEYWORD;
byte                      return KEYWORD;
char                      return KEYWORD;
double                    return KEYWORD;
float                     return KEYWORD;
int                       return KEYWORD;
long                      return KEYWORD;
short                     return KEYWORD;
size_t                    return KEYWORD;
string                    return KEYWORD;
String                    return KEYWORD;
void                      return KEYWORD;
word                      return KEYWORD;
const                     return KEYWORD;
scope                     return KEYWORD;
static                    return KEYWORD;
volatile                  return KEYWORD;
break                     return KEYWORD;
continue                  return KEYWORD;
do                        return KEYWORD;
else                      return KEYWORD;
for                       return KEYWORD;
goto                      return KEYWORD;
return                    return KEYWORD;
switch                    return KEYWORD;
while                     return KEYWORD;
HIGH                      return CONSTANTS;
LOW                       return CONSTANTS;
INPUT                     return CONSTANTS;
OUTPUT                    return CONSTANTS;
INPUT_PULLUP              return CONSTANTS;
LED_BUILTIN               return CONSTANTS;
true                      return CONSTANTS;
false                     return CONSTANTS;



[-+]?[0-9]+				  return CONSTANTLITERAL;

O[0-9]+					  return CONSTANTLITERAL;

OX[0-9abcdef]+			  return CONSTANTLITERAL;

[-+]?[0-9]+(\.[0-9]+)?    return CONSTANTLITERAL;

'([^\r^\n^']|\\0)'		  return CONSTANTCHAR;

\"[^\"]*\"                return CONSTANTSTRING;
			


[a-zA-Z_][_a-zA-Z0-9]*	  return IDENTIFIER;


\+						return OPERATOR;
- 						return OPERATOR;
\* 						return OPERATOR;
\/ 						return OPERATOR;
\%						return OPERATOR;
\+\+					return OPERATOR;
"--"					return OPERATOR;
					
					
==						return OPERATOR;
!=						return OPERATOR;
\> 						return OPERATOR;
\< 						return OPERATOR;
\>=						return OPERATOR;
\<=						return OPERATOR;


&&						return OPERATOR;
\|\|					return OPERATOR;
!						return OPERATOR;


& 						return OPERATOR;
\| 						return OPERATOR;
\^ 						return OPERATOR;
~ 						return OPERATOR;
\<\<					return OPERATOR;
\>\>					return OPERATOR;


"?"						return OPERATOR;


\(						return PUNCTUATION;
\)						return PUNCTUATION;
\{						return PUNCTUATION;
\}						return PUNCTUATION;
\[						return PUNCTUATION;
\]						return PUNCTUATION;
,						return PUNCTUATION;
:						return PUNCTUATION;
;						return PUNCTUATION;
=						return PUNCTUATION;



[ \t\n]					return BLANK;

.						return ERROR;

%%

int yywrap(void)
{
	return 1;
}