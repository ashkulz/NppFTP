#!/usr/bin/env python3

# --------------------------------------------------------------- CONFIGURATION

DEPENDENT_LIBS = {
    'openssl': {
        'order' : 1,
        'url'   : 'https://www.openssl.org/source/openssl-1.0.2j.tar.gz',
        'sha1'  : 'bdfbdb416942f666865fa48fe13c2d0e588df54f',
        'target': {
            'mingw-w64': {
                'result':   ['include/openssl/ssl.h', 'lib/libssl.a', 'lib/libcrypto.a'],
                'commands': [
                    'perl Configure --openssldir=%(dest)s --cross-compile-prefix=i686-w64-mingw32- no-shared no-asm mingw64',
                    'make depend', 'make', 'make install_sw'
                ]
            },
            'msvc': {
                'result':   ['include/openssl/ssl.h', 'lib/libeay32.lib', 'lib/ssleay32.lib'],
                'commands': [
                    'perl Configure --openssldir=%(dest)s no-asm VC-WIN32',
                    'ms\\do_ms.bat',
                    'nmake /f ms\\nt.mak install'
                ]
            }
        }
    },

    'zlib': {
        'order' : 2,
        'url'   : 'http://downloads.sourceforge.net/libpng/zlib-1.2.8.tar.gz',
        'sha1'  : 'a4d316c404ff54ca545ea71a27af7dbc29817088',
        'target': {
            'mingw-w64': {
                'result':   ['include/zlib.h', 'include/zconf.h', 'lib/libz.a'],
                'commands': [
                    'make -f win32/Makefile.gcc PREFIX=i686-w64-mingw32-',
                    'cp zlib.h zconf.h %(dest)s/include',
                    'cp libz.a %(dest)s/lib'
                ]
            },
            'msvc': {
                'result':   ['include/zlib.h', 'include/zconf.h', 'lib/zlib.lib'],
                'replace':  [('win32/Makefile.msc', '-MD', '-MT')],
                'commands': [
                    'nmake /f win32/Makefile.msc zlib.lib',
                    'copy /Y zlib.h   %(dest)s\\include >nul',
                    'copy /Y zconf.h  %(dest)s\\include >nul',
                    'copy /Y zlib.lib %(dest)s\\lib     >nul'
                ]
            }
        }
    },

    'libssh': {
        'order' : 3,
        'shadow': True,
        'url'   : 'https://red.libssh.org/attachments/download/195/libssh-0.7.3.tar.xz',
        'sha1'  : '9de2a8fde51aa7b7855008fafd5bf47ebb01289f',
        'target': {
            'mingw-w64': {
                'result':   ['include/libssh/libssh.h', 'lib/libssh.a'],
                'commands': [
                    'cmake -DCMAKE_SYSTEM_NAME=Windows \
                        -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
                        -DOPENSSL_INCLUDE_DIRS=%(dest)s/include -DOPENSSL_CRYPTO_LIBRARY=%(dest)s/lib/libcrypto.a \
                        -DWITH_STATIC_LIB=ON -DCMAKE_INSTALL_PREFIX=%(dest)s -DCMAKE_PREFIX_PATH=%(dest)s %(src)s',
                    'make',
                    'make install'
                ]
            },
            'msvc': {
                'result':   ['include/libssh/libssh.h', 'lib/ssh.lib'],
                'commands': [
                    'cmake -G "NMake Makefiles" -DWITH_STATIC_LIB=ON -DCMAKE_BUILD_TYPE=Release \
                        "-DCMAKE_C_FLAGS_RELEASE=/MT /O2 /Ob2 /D NDEBUG" "-DCMAKE_CXX_FLAGS_RELEASE=/MT /O2 /Ob2 /D NDEBUG" \
                        -DOPENSSL_INCLUDE_DIRS=%(dest)s\\include -DOPENSSL_CRYPTO_LIBRARY=%(dest)s\\lib\\libeay32.lib \
                        -DCMAKE_INSTALL_PREFIX=%(dest)s -DCMAKE_PREFIX_PATH=%(dest)s %(src)s',
                    'nmake install',
                    'del %(dest)s\\lib\\ssh.lib >nul',
                    'move %(dest)s\\lib\\static\\ssh.lib %(dest)s\\lib >nul'
                ]
            }

        }
    }
}

# --------------------------------------------------------------- HELPERS

import os, sys, platform, shutil, urllib.request, hashlib, tarfile

def join_path(*p):
    return os.path.abspath(os.path.join(*p))

def message(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

def error(msg):
    message(msg+'\n')
    sys.exit(1)

def shell(cmd):
    ret = os.system(cmd)
    if ret != 0:
        error("%s\ncommand failed: exit code %d" % (cmd, ret))

def rmdir(path):
    if os.path.exists(path):
        if platform.system() == 'Windows':
            shell('attrib -R %s\* /S' % path)
        shutil.rmtree(path)

def mkdir_p(*paths):
    path = join_path(*paths)
    if not os.path.exists(path):
        os.makedirs(path)

def download_file(url, sha1, dir):
    name = url.split('/')[-1]
    loc  = join_path(dir, name)
    if os.path.exists(loc):
        hash = hashlib.sha1(open(loc, 'rb').read()).hexdigest()
        if hash == sha1:
            return loc
        os.remove(loc)
        message('Checksum mismatch for %s, re-downloading.\n' % name)
    def hook(cnt, bs, total):
        pct = int(cnt*bs*100/total)
        message("\rDownloading: %s [%d%%]" % (name, pct))
    urllib.request.urlretrieve(url, loc, reporthook=hook)
    message("\r")
    hash = hashlib.sha1(open(loc, 'rb').read()).hexdigest()
    if hash != sha1:
        os.remove(loc)
        error('Checksum mismatch for %s, aborting.' % name)
    message("\rDownloaded: %s [checksum OK]\n" % name)
    return loc

def download_tarball(url, sha1, dir, name):
    loc = download_file(url, sha1, dir)
    tar = tarfile.open(loc)
    sub = tar.getnames()[0]
    if '/' in sub:
        sub = sub[:sub.index('/')]
    src = join_path(dir, sub)
    tgt = join_path(dir, name)
    rmdir(src)
    tar.extractall(dir)
    rmdir(tgt)
    os.rename(src, tgt)
    return tgt

# --------------------------------------------------------------- BUILDING

def main():
    build = os.path.abspath('obj')
    dest  = os.path.abspath('3rdparty')
    mkdir_p(build)
    mkdir_p(dest, 'include')
    mkdir_p(dest, 'lib')

    target = platform.system() == 'Windows' and 'msvc' or 'mingw-w64'
    for library in sorted(DEPENDENT_LIBS, key=lambda x: DEPENDENT_LIBS[x]['order']):
        if target not in DEPENDENT_LIBS[library]['target']:
            print('%s: skipping (not available)' % library)
            continue

        cfg = DEPENDENT_LIBS[library]['target'][target]
        built = True
        for path in cfg['result']:
            built = built and os.path.exists(join_path(dest, path))
        if built:
            print('%s: skipping (already built)' % library)
            continue
        print('-' * 60, library)
        download_tarball(DEPENDENT_LIBS[library]['url'],
                         DEPENDENT_LIBS[library]['sha1'], build, library)

        if DEPENDENT_LIBS[library].get('shadow'):
            rmdir(join_path(build, library+'-build'))
            mkdir_p(join_path(build, library+'-build'))
            os.chdir(join_path(build, library+'-build'))
        else:
            os.chdir(join_path(build, library))

        for location, src, tgt in cfg.get('replace', []):
            data = open(join_path(build, library, location), 'r').read()
            open(join_path(build, library, location), 'w').write(data.replace(src, tgt))

        for cmd in cfg['commands']:
            shell(cmd % { 'dest' : dest, 'src': join_path(build, library) })
        os.chdir(dest)
        for path in cfg['result']:
            if not os.path.exists(path):
                error('Unable to build %s, missing: %s' % (library, path))

if __name__ == '__main__':
    main()
