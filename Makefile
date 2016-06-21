!IF "$(PLATFORM)"=="X64"
OUTDIR=bin64
OBJDIR=obj64
ARCH=amd64
!ELSE
OUTDIR=bin
OBJDIR=obj
ARCH=x86
!ENDIF

CC=cl
LINKER=link
RM=del /q

TARGET=minib.exe

OBJS=\
    $(OBJDIR)\minibrowser.res\
    $(OBJDIR)\minihost.obj\
    $(OBJDIR)\main.obj\

LIBS=\
    user32.lib\
    shcore.lib\

# C4100: unreferenced parameter
CFLAGS=\
    /nologo\
    /Zi\
    /c\
    /Fo"$(OBJDIR)\\"\
    /Fd"$(OBJDIR)\\"\
    /D_UNICODE\
    /DUNICODE\
    /O2\
    /EHsc\
    /W4\
    /wd4100\

LFLAGS=\
    /NOLOGO\
    /DEBUG\
    /SUBSYSTEM:WINDOWS\
    /INCREMENTAL:NO\
!IF "$(PLATFORM)"=="X64"
    /MACHINE:X64\
!ELSE
    /MACHINE:X86\
!ENDIF

all: clean dirs $(OUTDIR)\$(TARGET)

clean:
    -@if not exist $(OBJDIR) md $(OBJDIR)
    @$(RM) /Q $(OBJDIR)\* 2>nul

dirs:
    @if not exist $(OBJDIR) mkdir $(OBJDIR)
    @if not exist $(OUTDIR) mkdir $(OUTDIR)

$(OUTDIR)\$(TARGET): $(OBJS)
    $(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**
    MT -nologo -manifest res\manifest.xml\
       -identity:"Minib, type=win32, version=1.0.0.0, processorArchitecture=$(ARCH)"\
       -outputresource:$@;1

.cpp{$(OBJDIR)}.obj:
    $(CC) $(CFLAGS) $<

.rc{$(OBJDIR)}.res:
    rc /nologo /fo "$@" $<
