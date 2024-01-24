#!/usr/bin/env python3

# --------------------------------------------------------------- CONFIGURATION

DEPENDENT_LIBS = {
    'openssl': {
        'order' : 1,
        'url'   : 'https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz',
        'sha1'  : '740916d79ab0d209d2775277b1c6c3ec2f6502b2',
        'target': {
            'mingw-w64': {
                'result':   ['include/openssl/ssl.h', 'lib/libssl.a', 'lib/libcrypto.a'],
                'commands': [
                    'perl Configure --openssldir=%(dest)s --cross-compile-prefix=%(prefix)s- no-shared no-asm mingw64',
                    'make depend', 'make', 'make install_sw'
                ]
            },
            'msvc': {
                'result':   ['include/openssl/ssl.h', 'lib/libeay32.lib', 'lib/ssleay32.lib'],
                'commands': [
                    'perl Configure --openssldir=%(dest)s no-shared no-asm VC-WIN32 -wd4005',
                    'ms\\do_ms.bat',
                    'nmake /f ms\\nt.mak install'
                ]
            },
            'msvc_x64': {
                'result':   ['include/openssl/ssl.h', 'lib/libeay32.lib', 'lib/ssleay32.lib'],
                'commands': [
                    'perl Configure --openssldir=%(dest)s no-shared no-asm VC-WIN64A',
                    'ms\\do_win64a.bat',
                    'nmake /f ms\\nt.mak install'
                ]
            }
        }
    },

    'zlib': {
        'order' : 2,
        'url'   : 'https://zlib.net/fossils/zlib-1.3.1.tar.gz',
        'sha1'  : 'f535367b1a11e2f9ac3bec723fb007fbc0d189e5',
        'target': {
            'mingw-w64': {
                'result':   ['include/zlib.h', 'include/zconf.h', 'lib/libz.a'],
                'commands': [
                    'make -f win32/Makefile.gcc PREFIX=%(prefix)s-',
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
            },
            'msvc_x64': {
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
        'url'   : 'https://www.libssh.org/files/0.10/libssh-0.10.6.tar.xz',
        'sha1'  : 'e8fb3b4750db11d2483cac4b5f046e301c09b72f',
        'target': {
            'mingw-w64': {
                'result':   ['include/libssh/libssh.h', 'lib/libssh.a'],
                'commands': [
                    'cmake -DCMAKE_SYSTEM_NAME=Windows \
                        -DCMAKE_C_COMPILER=%(prefix)s-gcc -DCMAKE_CXX_COMPILER=%(prefix)s-g++ \
                        -DOPENSSL_INCLUDE_DIRS=%(dest)s/include -DOPENSSL_CRYPTO_LIBRARY=%(dest)s/lib/libcrypto.a \
                        -DBUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=OFF -DWITH_EXAMPLES=OFF -DWITH_SERVER=OFF -DCMAKE_INSTALL_PREFIX=%(dest)s -DCMAKE_PREFIX_PATH=%(dest)s %(src)s',
                    'make',
                    'make install'
                ]
            },
            'msvc': {
                'result':   ['include/libssh/libssh.h', 'lib/ssh.lib'],
                'commands': [
                    'cmake -G "NMake Makefiles" -DBUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=OFF -DWITH_EXAMPLES=OFF -DWITH_SERVER=OFF -DCMAKE_BUILD_TYPE=Release \
                        "-DCMAKE_C_FLAGS_RELEASE=/MP /MT /O2 /Ob2 /D NDEBUG" "-DCMAKE_CXX_FLAGS_RELEASE=/MP /MT /O2 /Ob2 /D NDEBUG" \
                        -DOPENSSL_INCLUDE_DIRS=%(dest)s\\include -DOPENSSL_CRYPTO_LIBRARY=%(dest)s\\lib\\libeay32.lib \
                        -DCMAKE_INSTALL_PREFIX=%(dest)s -DCMAKE_PREFIX_PATH=%(dest)s %(src)s',
                    'nmake install'
                ]
            },
            'msvc_x64': {
                'result':   ['include/libssh/libssh.h', 'lib/ssh.lib'],
                'commands': [
                    'cmake -G "NMake Makefiles" -DBUILD_STATIC_LIB=ON -DBUILD_SHARED_LIBS=OFF -DWITH_EXAMPLES=OFF -DWITH_SERVER=OFF -DCMAKE_BUILD_TYPE=Release \
                        "-DCMAKE_C_FLAGS_RELEASE=/MP /MT /O2 /Ob2 /D NDEBUG" "-DCMAKE_CXX_FLAGS_RELEASE=/MP /MT /O2 /Ob2 /D NDEBUG" \
                        -DOPENSSL_INCLUDE_DIRS=%(dest)s\\include -DOPENSSL_CRYPTO_LIBRARY=%(dest)s\\lib\\libeay32.lib \
                        -DCMAKE_INSTALL_PREFIX=%(dest)s -DCMAKE_PREFIX_PATH=%(dest)s %(src)s',
                    'nmake install'
                ]
            }
        }
    }
}

# --------------------------------------------------------------- HELPERS

import os, sys, platform, shutil, urllib.request, hashlib, tarfile, subprocess

def join_path(*p):
    return os.path.abspath(os.path.join(*p))

def message(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

def error(msg):
    message(msg+'\n')
    sys.exit(1)

def shell(cmd):
    message('    %s\n' % cmd)
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError as e:
        error("\n%s\ncommand failed: exit code %d" % (e.output.decode('utf-8'), e.returncode))

def rmdir(path):
    if os.path.exists(path):
        if platform.system() == 'Windows':
            shell('attrib -R %s\\* /S' % path)
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

def main(outdir, prefix):
    build = os.path.abspath(os.path.join(outdir, 'obj'))
    dest  = os.path.abspath(os.path.join(outdir, '3rdparty'))
    mkdir_p(build)
    mkdir_p(dest, 'include')
    mkdir_p(dest, 'lib')

    if(prefix == 'msvc_x64'):
        target = 'msvc_x64'
    else:
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
            shell(cmd % { 'dest' : dest, 'src': join_path(build, library), 'prefix': prefix })
        os.chdir(dest)
        for path in cfg['result']:
            if not os.path.exists(path):
                error('Unable to build %s, missing: %s' % (library, path))

if __name__ == '__main__':
    outdir = sys.argv[1] if len(sys.argv) in (2,3) else 'x86'
    prefix = sys.argv[2] if len(sys.argv) ==    3  else ''
    main(outdir, prefix)
