# $Id: SConstruct,v 1.1 2004/02/07 02:01:29 fab Exp $
import sys
prefix = '/usr/local'
env = Environment()

env.Append(CPPDEFINES={'GTK2':1, 'LOCALEDIR':'\\"'+prefix+'/share/locale\\"'})
env.Append(CCFLAGS=['-ggdb','-ansi','-Wall'])
#env.Append(LINKFLAGS='`pkg-config --libs gtk+-2.0`')

Export('env')
#SConscript(['src/SConscript','docs/SConscript'])
SConscript(['src/SConscript'])
