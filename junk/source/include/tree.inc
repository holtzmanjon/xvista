


C       DEFINE ARRAYS TO FORM A LINKED LIST FOR THE PARSER

        INTEGER TSIZE
        PARAMETER (TSIZE=500)

        INTEGER*4 BACK(TSIZE),FORW(TSIZE),SYMS(TSIZE),SYME(TSIZE)
        INTEGER*4 WTYPE(TSIZE)
        DOUBLE PRECISION QVAL(TSIZE)
        INTEGER TKNT,SYMKNT
        CHARACTER*4000 SYMSAVE

        COMMON/TREESAVE/BACK,FORW,SYMS,SYME,QVAL,WTYPE,TKNT,SYMKNT
        COMMON/TREESYM/SYMSAVE

C       DEFINE PARSER TYPES
        INTEGER TREEEND,TNUM,TMUL,TDIV,TADD,TSUB,TPOS
        INTEGER TNEG,TSYM,TLBR,TRBR,TBINARY,TUNARY
        INTEGER TEQU,TFUN,TFEN,TSEP,TEXP,TVIR
        INTEGER TSAME,TGE,TGT,TLE,TLT,TNE,TOR,TAND

        PARAMETER (TREEEND=0)
        PARAMETER (TNUM=0)
C               ! A NUMERIC VALUE
        PARAMETER (TMUL=1)
C               ! MULTIPLY OPERATOR (BINARY)
        PARAMETER (TDIV=2)
C               ! DIVIDE OPERATOR
        PARAMETER (TADD=3)
C               ! ADDITION OPERATOR
        PARAMETER (TSUB=4)
C               ! SUBTRACTION OPERATOR
        PARAMETER (TPOS=5)
C               ! POSITIVE OPERATOR (UNARY)
        PARAMETER (TNEG=6)
C               ! NEGATION OPERATOR (UNARY)
        PARAMETER (TSYM=7)
C               ! SYMBOL
        PARAMETER (TLBR=8)
C               ! LEFT BRACKET
        PARAMETER (TRBR=9)
C               ! RIGHT BRACKET
        PARAMETER (TEQU=10)
C               ! EQUATE OPERATOR
        PARAMETER (TFUN=11)
C               ! START OF FUNCTION PARAMETER LIST
        PARAMETER (TFEN=12)
C               ! END OF FUNCTION PARAMETER LIST
        PARAMETER (TSEP=13)
C               ! FUNCTION PARAMETER SEPARATOR
        PARAMETER (TEXP=14)
C               ! EXPONENTIATION
        PARAMETER (TSAME=15)
C               ! LOGICAL EQUAL
        PARAMETER (TGE=16)
C               ! LOGICAL GREATER THAN OR EQUAL
        PARAMETER (TGT=17)
C               ! LOGICAL GREATER THAN
        PARAMETER (TLE=18)
C               ! LOGICAL LESS THAN OR EQUAL
        PARAMETER (TLT=19)
C               ! LOGICAL LESS THAN
        PARAMETER (TNE=20)
C               ! LOGICAL NOT EQUAL
        PARAMETER (TOR=21)
C               ! LOGICAL OR
        PARAMETER (TAND=22)
C               ! LOGICAL AND
        PARAMETER (TVIR=23)
C               ! Virtual address (array element)

        PARAMETER (TBINARY=100)
C               ! USED TO CLASSIFY BINARY OPERATORS
        PARAMETER (TUNARY=101)
C               ! USED TO CLASSIFY UNARY OPERATORS
