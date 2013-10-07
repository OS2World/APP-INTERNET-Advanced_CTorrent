ACTVERSION=[ACT 0.2.7 t2]
DESC = 'ctorrent dnh3.3.2 [ACT 0.2.7 t2]'
ObjDir = .\obj
ExeDir = .

#Compiler_Options = -zq -zp=1 -DERRLOG
#Compiler_Options = -zq -zp=1
#Linker_options   = 
Compiler_Options = -wx -bm -zq -zp=1 -obmilert -DACTVERSION="$(ACTVERSION)" -DERFILES -d2
#Compiler_Options = -br -wx -bm -zq -zp=1 -obmilert -DACTVERSION="$(ACTVERSION)" -DERFILES
#Compiler_Options = -br -zq -zp=1 -obmilert -DACTVERSION="$(ACTVERSION)"
#Linker_options   = option statics LIBRARY libconv.lib
Linker_options   = LIBRARY libconv.lib option stack=250k option map 
#DEBUG ALL
#Linker_options   = LIBRARY libconv.lib option map
System       = os2v2

ExeFile     = act.exe

Object_files =  $(ObjDir)\ctorrent.obj &
#		$(ObjDir)\dbgheap.obj &
		$(ObjDir)\bencode.obj &
		$(ObjDir)\bitfield.obj &
		$(ObjDir)\btconfig.obj &
		$(ObjDir)\btcontent.obj &
		$(ObjDir)\btfiles.obj &
		$(ObjDir)\btrequest.obj &
		$(ObjDir)\btstream.obj &
		$(ObjDir)\bufio.obj &
		$(ObjDir)\compat.obj &
		$(ObjDir)\connect_nonb.obj &
		$(ObjDir)\console.obj &
		$(ObjDir)\ctcs.obj &
		$(ObjDir)\downloader.obj &
		$(ObjDir)\httpencode.obj &
		$(ObjDir)\iplist.obj &
		$(ObjDir)\peer.obj &
		$(ObjDir)\peerlist.obj &
		$(ObjDir)\rate.obj &
		$(ObjDir)\setnonblock.obj &
		$(ObjDir)\sigint.obj &
		$(ObjDir)\tracker.obj &
		$(ObjDir)\sha1.obj &
		$(ObjDir)\erfile.obj &
		$(ObjDir)\acw.obj &
		$(ObjDir)\my_iconv.obj &
		$(ObjDir)\getopts.obj &
		$(ObjDir)\Parser.obj

.ERASE

#$(ExeDir)\$(ExeFile) : $(Object_files) makefile .AUTODEPEND
$(ExeDir)\$(ExeFile) : $(Object_files) .AUTODEPEND
 wlink system $(System) $(Linker_Options) name $(ExeDir)\$(ExeFile) file {$(Object_files)} OPTION DE $(DESC)
 rc16 -p -x act.res $@
# lxlite $(ExeDir)\$(ExeFile)

.CPP.OBJ: .AUTODEPEND
 WPP386 $(Compiler_Options) -fo$@ $<

.C.OBJ: .AUTODEPEND
 WCC386 $(Compiler_Options) -fo$@ $<

