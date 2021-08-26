
# Notes

 - Numbers are usually 2 bytes in big endian form
 
 - text:
	--first byte-------   --second byte---
    7    6 5 4 3 2  1 0   7 6 5  4 3 2 1 0
    bit  --first--  --second---  --third--

 - ZSCII
 
   Z-char 6789abcdef0123456789abcdef
current   --------------------------
  A0      abcdefghijklmnopqrstuvwxyz
  A1      ABCDEFGHIJKLMNOPQRSTUVWXYZ
  A2       ^0123456789.,!?_#'"/\-:()
          --------------------------
Ref: http://inform-fiction.org/zmachine/standards/z1point1/sect03.html

## Memory

Ref: http://inform-fiction.org/zmachine/standards/z1point1/sect01.html

 - 64 bytes of header
 - dynamic memory
 - static memory
 - high memory
 
 - packet address:
  2P           Versions 1, 2 and 3
  4P           Versions 4 and 5
  4P + 8R_O    Versions 6 and 7, for routine calls
  4P + 8S_O    Versions 6 and 7, for print_paddr
  8P           Version 8

## Instructions

 If the top two bits of the opcode are $$11 the form is variable; if $$10, the form is short. If the opcode is 190 ($BE in hexadecimal) and the version is 5 or later, the form is "extended". Otherwise, the form is "long".
 
 In variable form, if bit 5 is 0 then the count is 2OP; if it is 1, then the count is VAR. The opcode number is given in the bottom 5 bits.
 
 In variable or extended forms, a byte of 4 operand types is given next. This contains 4 2-bit fields: bits 6 and 7 are the first field, bits 0 and 1 the fourth. The values are operand types as above. Once one type has been given as 'omitted', all subsequent ones must be. Example: $$00101111 means large constant followed by variable (and no third or fourth opcode).
 
 There are four 'types' of operand. These are often specified by a number stored in 2 binary digits:

  $$00    Large constant (0 to 65535)    2 bytes
  $$01    Small constant (0 to 255)      1 byte
  $$10    Variable                       1 byte
  $$11    Omitted altogether             0 bytes
  
 "Store" instructions return a value: e.g., mul multiplies its two operands together. Such instructions must be followed by a single byte giving the variable number of where to put the result.
 
E0 => 11100000 => 11 (variable form), 1 (count is VAR), opcode = 0 (call routine ...0 to 3 args... -> (result))
03 => 00000011 => 00 (large constant), 00 (large constant), 00 (large constant), 11 (nothing)
2A FD
83 A4
FF FF
00 => variable number for result

CALL:

 - first op is **paked** address of routine
 
 A packed address specifies where a routine or string begins in high memory. Given a packed address P, the formula to obtain the corresponding byte address B is:

  2P           Versions 1, 2 and 3
  4P           Versions 4 and 5
  4P + 8R_O    Versions 6 and 7, for routine calls
  4P + 8S_O    Versions 6 and 7, for print_paddr
  8P           Version 8
R_O and S_O are the routine and strings offsets (specified in the header as words at $28 and $2a, respectively).


Variable number $00 refers to the top of the stack, $01 to $0f mean the local variables of the current routine and $10 to $ff mean the global variables


E0 03 2A 39 80 10 FF FF
00 E1 97 00 00 01 E0 03
2A 39 80 7C FF FF 00 E0
03 2A 39 80 F0 FF FF 00

In short form, bits 4 and 5 of the opcode byte give an operand type as above. If this is $11 then the operand count is 0OP; otherwise, 1OP. In either case the opcode number is given in the bottom 4 bits.

54 94 B4 03 74 94 92 04
61 04 03 58 55 92 06 92
A0 02 C6 55 93 06 93 74
94 92 05 E1 9B 05 02 01

54 = 01010100: 0 => long form (so 2OP), 1 = var, 0 = const, 10100 = h14 = ADD
ADD = h14 00010100, Signed 16-bit addition

In long form the operand count is always 2OP. The opcode number is given in the bottom 5 bits.

In long form, bit 6 of the opcode gives the type of the first operand, bit 5 of the second.
A value of 0 means a small constant and 1 means a variable. (If a 2OP instruction needs a
large constant as operand, then it should be assembled in variable rather than long form.)


https://docs.google.com/spreadsheets/d/1opwRfbMi5Asx3oFWNhAhVJ-R1RuUrV23VgPpbSAco4E/edit#gid=0


# Notes from icculus

These are the instructions you need to implement to get to the title/copyright:

[X] add
[X] call
[X] je
jump
[X] jz
loadw
print
put_prop
[X] ret
store
[X] storew
[X] sub
test_attr
new_line

...these are all the opcodes that Zork 1 uses, below. Of all the 119 opcodes,
not all are available to the version 3 Z-Machine, but Zork doesn't use even
all of those:

ADD
AND
CALL
CLEAR_ATTR
DEC
DEC_CHK
DIV
GET_CHILD
GET_NEXT_PROP
GET_PARENT
GET_PROP
GET_PROP_ADDR
GET_PROP_LEN
GET_SIBLING
INC
INC_CHK
INSERT_OBJ
JE
JG
JIN
JL
JUMP
JZ
LOAD
LOADB
LOADW
MOD
MUL
NEW_LINE
OR
PRINT
PRINT_ADDR
PRINT_CHAR
PRINT_NUM
PRINT_OBJ
PRINT_PADDR
PRINT_RET
PULL
PUSH
PUT_PROP
QUIT
RANDOM
READ
REMOVE_OBJ
RESTART
RESTORE
RET
RET_POPPED
RFALSE
RTRUE
SAVE
SET_ATTR
STORE
STOREB
STOREW
SUB
TEST
TEST_ATTR
VERIFY


# References

 - [Zork 1 data files](https://github.com/the-infocom-files/zork1)
 - [Z-Machine standards](http://inform-fiction.org/zmachine/standards/)
 - [icculus notes](https://github.com/icculus/mojozork/blob/3c486a9e11a6959afdd6ede9e65f2c8cbf33e5d3/notes.txt#L2-L17)
