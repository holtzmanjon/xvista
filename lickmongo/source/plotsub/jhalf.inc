C       ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
C       This Fortran module is Copyrighted software.
C       The file COPYRIGHT must accompany this file.  See it for details.
C       ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
C
C   Block data for bit-reversing, plotting characters, etc.
C

	INTEGER         bits(0:31)
	DATA            bits
     1  / '00000001'X, '00000002'X, '00000004'X, '00000008'X,
     2    '00000010'X, '00000020'X, '00000040'X, '00000080'X,
     3    '00000100'X, '00000200'X, '00000400'X, '00000800'X,
     4    '00001000'X, '00002000'X, '00004000'X, '00008000'X,
     5    '00010000'X, '00020000'X, '00040000'X, '00080000'X,
     6    '00100000'X, '00200000'X, '00400000'X, '00800000'X,
     7    '01000000'X, '02000000'X, '04000000'X, '08000000'X,
     8    '10000000'X, '20000000'X, '40000000'X, '80000000'X/

	INTEGER         bitrev5(0:31)
	DATA            bitrev5
     1  / 0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,
     2   29,3,19,11,27,7,23,15,31/

	INTEGER         bitrev8(-128:127)
	DATA            bitrev8
     1 / 1,-127,65,-63,33,-95,97,-31,17,-111,
     &  81,-47,49,-79,113,-15,9,-119,
     2  73,-55,41,-87,105,-23,25,-103,89,-39,57,-71,121,-7,5,-123,69,
     3  -59,37,-91,101,-27,21,-107,85,-43,53,-75,117,-11,13,-115,77,-51,
     4  45,-83,109,-19,29,-99,93,-35,61,-67,125,-3,3,-125,67,-61,35,-93,
     5  99,-29,19,-109,83,-45,51,-77,115,-13,11,-117,75,-53,43,-85,107,
     6  -21,27,-101,91,-37,59,-69,123,-5,7,-121,71,-57,39,-89,103,-25,
     7  23,-105,87,-41,55,-73,119,-9,15,-113,79,-49,47,-81,111,-17,31,
     8  -97,95,-33,63,-65,127,-1,0,-128,64,-64,32,-96,96,-32,16,-112,
     9  80,-48,48,-80,112,-16,8,-120,72,-56,40,-88,104,-24,24,-104,88,
     1  -40,56,-72,120,-8,4,-124,68,-60,36,-92,100,-28,20,-108,84,-44,
     1  52,-76,116,-12,12,-116,76,-52,44,-84,108,-20,28,-100,92,-36,60,
     2  -68,124,-4,2,-126,66,-62,34,-94,98,-30,18,-110,82,-46,50,-78,
     3  114,-14,10,-118,74,-54,42,-86,106,-22,26,-102,90,-38,58,-70,122,
     4  -6,6,-122,70,-58,38,-90,102,-26,22,-106,86,-42,54,-74,118,-10,
     5  14,-114,78,-50,46,-82,110,-18,30,-98,94,-34,62,-66,126,-2 /

	COMMON /halftbls/ bits, bitrev5, bitrev8
