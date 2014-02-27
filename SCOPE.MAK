#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCCDOS  = Bcc +BccDos.cfg 
TLINK   = TLink
TLIB    = TLib
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGSDOS =  -LD:\BC45\LIB
IDE_BFLAGS = 
LLATDOS_scopedexe =  -c -Tde -LC:\BC45\LIB;D:\BC45\LIB -P=8192
RLATDOS_scopedexe = 
BLATDOS_scopedexe = 
CNIEAT_scopedexe = -IC:\BC45\INCLUDE;D:\BC45\INCLUDE -D
LNIEAT_scopedexe = -x
LEAT_scopedexe = $(LLATDOS_scopedexe)
REAT_scopedexe = $(RLATDOS_scopedexe)
BEAT_scopedexe = $(BLATDOS_scopedexe)
CLATW16_Ccbbc45blibbgraphicsdlib = 
LLATW16_Ccbbc45blibbgraphicsdlib = 
RLATW16_Ccbbc45blibbgraphicsdlib = 
BLATW16_Ccbbc45blibbgraphicsdlib = 
CEAT_Ccbbc45blibbgraphicsdlib = $(CEAT_scopedexe) $(CLATW16_Ccbbc45blibbgraphicsdlib)
CNIEAT_Ccbbc45blibbgraphicsdlib = -IC:\BC45\INCLUDE;D:\BC45\INCLUDE -D
LNIEAT_Ccbbc45blibbgraphicsdlib = -x
LEAT_Ccbbc45blibbgraphicsdlib = $(LEAT_scopedexe) $(LLATW16_Ccbbc45blibbgraphicsdlib)
REAT_Ccbbc45blibbgraphicsdlib = $(REAT_scopedexe) $(RLATW16_Ccbbc45blibbgraphicsdlib)
BEAT_Ccbbc45blibbgraphicsdlib = $(BEAT_scopedexe) $(BLATW16_Ccbbc45blibbgraphicsdlib)

#
# Dependency List
#
Dep_scope = \
   scope.exe

scope : BccDos.cfg $(Dep_scope)
  echo MakeNode 

Dep_scopedexe = \
   altazec.obj\
   common.obj\
   config.obj\
   convert.obj\
   converta.obj\
   encoders.obj\
   guide.obj\
   handpad.obj\
   hpproc.obj\
   iaca.obj\
   kbdfile.obj\
   kbencodr.obj\
   kbguide.obj\
   kbinproc.obj\
   kbmain.obj\
   kbproc.obj\
   link.obj\
   lx200.obj\
   mevent.obj\
   pec.obj\
   scope.obj\
   steppers.obj\
   track.obj\
   egavga.obj\
   C:\bc45\lib\graphics.lib

scope.exe : $(Dep_scopedexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGSDOS) $(LEAT_scopedexe) $(LNIEAT_scopedexe) +
C:\BC45\LIB\c0l.obj+
altazec.obj+
common.obj+
config.obj+
convert.obj+
converta.obj+
encoders.obj+
guide.obj+
handpad.obj+
hpproc.obj+
iaca.obj+
kbdfile.obj+
kbencodr.obj+
kbguide.obj+
kbinproc.obj+
kbmain.obj+
kbproc.obj+
link.obj+
lx200.obj+
mevent.obj+
pec.obj+
scope.obj+
steppers.obj+
track.obj+
egavga.obj
$<,$*
C:\bc45\lib\graphics.lib+
C:\BC45\LIB\bidsl.lib+
C:\BC45\LIB\emu.lib+
C:\BC45\LIB\mathl.lib+
C:\BC45\LIB\cl.lib

|

altazec.obj :  altazec.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ altazec.c
|

common.obj :  common.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ common.c
|

config.obj :  config.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ config.c
|

convert.obj :  convert.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ convert.c
|

converta.obj :  converta.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ converta.c
|

encoders.obj :  encoders.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ encoders.c
|

guide.obj :  guide.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ guide.c
|

handpad.obj :  handpad.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ handpad.c
|

hpproc.obj :  hpproc.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ hpproc.c
|

iaca.obj :  iaca.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ iaca.c
|

kbdfile.obj :  kbdfile.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbdfile.c
|

kbencodr.obj :  kbencodr.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbencodr.c
|

kbguide.obj :  kbguide.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbguide.c
|

kbinproc.obj :  kbinproc.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbinproc.c
|

kbmain.obj :  kbmain.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbmain.c
|

kbproc.obj :  kbproc.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ kbproc.c
|

link.obj :  link.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ link.c
|

lx200.obj :  lx200.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ lx200.c
|

mevent.obj :  mevent.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ mevent.c
|

pec.obj :  pec.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ pec.c
|

scope.obj :  scope.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ scope.c
|

steppers.obj :  steppers.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ steppers.c
|

track.obj :  track.c
  $(BCCDOS) -P- -c @&&|
 $(CEAT_scopedexe) $(CNIEAT_scopedexe) -o$@ track.c
|

# Compiler configuration file
BccDos.cfg : 
   Copy &&|
-W-
-R
-v
-vi
-H
-H=scope.csm
-ml
-f
-H
-Ot
-Og
-A-
-v
-R
-Z-
-O-
-O-e
-O-l
-O-b
-O-W
-Om
-O-p
-Oi
-Ov
-y-
-k-
-Fc
-j100
-2
-Ff=32767
-Ff-
| $@


