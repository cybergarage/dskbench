# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=asbench - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to asbench - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "asbench - Win32 Release" && "$(CFG)" !=\
 "asbench - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "asbench.mak" CFG="asbench - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "asbench - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "asbench - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "asbench - Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "asbench - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\asbench.exe"

CLEAN : 
	-@erase "$(INTDIR)\ASBENCH.OBJ"
	-@erase "$(INTDIR)\random.obj"
	-@erase "$(INTDIR)\WN32SCSI.OBJ"
	-@erase "$(OUTDIR)\asbench.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/asbench.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/asbench.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib crtdll.lib /nologo /subsystem:console /machine:I386 /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib crtdll.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/asbench.pdb" /machine:I386 /nodefaultlib\
 /out:"$(OUTDIR)/asbench.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ASBENCH.OBJ" \
	"$(INTDIR)\random.obj" \
	"$(INTDIR)\WN32SCSI.OBJ" \
	".\WNASPI32.LIB"

"$(OUTDIR)\asbench.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\asbench.exe" "$(OUTDIR)\asbench.bsc"

CLEAN : 
	-@erase "$(INTDIR)\ASBENCH.OBJ"
	-@erase "$(INTDIR)\ASBENCH.SBR"
	-@erase "$(INTDIR)\random.obj"
	-@erase "$(INTDIR)\random.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WN32SCSI.OBJ"
	-@erase "$(INTDIR)\WN32SCSI.SBR"
	-@erase "$(OUTDIR)\asbench.bsc"
	-@erase "$(OUTDIR)\asbench.exe"
	-@erase "$(OUTDIR)\asbench.ilk"
	-@erase "$(OUTDIR)\asbench.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/asbench.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/asbench.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ASBENCH.SBR" \
	"$(INTDIR)\random.sbr" \
	"$(INTDIR)\WN32SCSI.SBR"

"$(OUTDIR)\asbench.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib crtdll.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib crtdll.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/asbench.pdb" /debug /machine:I386 /nodefaultlib\
 /out:"$(OUTDIR)/asbench.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ASBENCH.OBJ" \
	"$(INTDIR)\random.obj" \
	"$(INTDIR)\WN32SCSI.OBJ" \
	".\WNASPI32.LIB"

"$(OUTDIR)\asbench.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "asbench - Win32 Release"
# Name "asbench - Win32 Debug"

!IF  "$(CFG)" == "asbench - Win32 Release"

!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ASBENCH.C
DEP_CPP_ASBEN=\
	".\aspi.h"\
	

!IF  "$(CFG)" == "asbench - Win32 Release"


"$(INTDIR)\ASBENCH.OBJ" : $(SOURCE) $(DEP_CPP_ASBEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"


"$(INTDIR)\ASBENCH.OBJ" : $(SOURCE) $(DEP_CPP_ASBEN) "$(INTDIR)"

"$(INTDIR)\ASBENCH.SBR" : $(SOURCE) $(DEP_CPP_ASBEN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random.c

!IF  "$(CFG)" == "asbench - Win32 Release"


"$(INTDIR)\random.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"


"$(INTDIR)\random.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\random.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WN32SCSI.C
DEP_CPP_WN32S=\
	".\aspi.h"\
	".\scsidefs.h"\
	".\wnaspi32.h"\
	

!IF  "$(CFG)" == "asbench - Win32 Release"


"$(INTDIR)\WN32SCSI.OBJ" : $(SOURCE) $(DEP_CPP_WN32S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"


"$(INTDIR)\WN32SCSI.OBJ" : $(SOURCE) $(DEP_CPP_WN32S) "$(INTDIR)"

"$(INTDIR)\WN32SCSI.SBR" : $(SOURCE) $(DEP_CPP_WN32S) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WNASPI32.LIB

!IF  "$(CFG)" == "asbench - Win32 Release"

!ELSEIF  "$(CFG)" == "asbench - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
