#!/usr/bin/env python

# --------------------------------------------------------------- CONFIGURATION

DEPENDENT_LIBS = {
    'openssl': {
        'order' : 1,
        'url'   : 'https://openssl.org/source/openssl-1.0.2c.tar.gz',
        'sha1'  : '6e4a5e91159eb32383296c7c83ac0e59b83a0a44',
        'target': {
            'mingw-w64': {
                'result':   ['include/openssl/ssl.h', 'lib/libssl.a', 'lib/libcrypto.a'],
                # fix CMake <= 3.2 detection, see https://github.com/Kitware/CMake/commit/c5d9a8283cfac15b4a5a07f18d5eb10c1f388505
                'replace':  [('crypto/opensslv.h', '# define OPENSSL_VERSION_NUMBER', '#define OPENSSL_VERSION_NUMBER')],
                'commands': [
                    'perl Configure --openssldir=%(dest)s --cross-compile-prefix=i686-w64-mingw32- no-shared no-asm mingw64',
                    'make',
                    'make install_sw'
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
            }
        }
    },

    'libssh': {
        'order' : 3,
        'shadow': True,
        'url'   : 'https://git.libssh.org/projects/libssh.git/snapshot/libssh-libssh-0.7.0.tar.gz',
        'sha1'  : 'a3099b960108f3be4ef1c85be38cb3c3be82e3ee',
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
            }
        }
    }
}

# --------------------------------------------------------------- HELPERS

import os, sys, platform, shutil, urllib, hashlib, tarfile

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
    urllib.urlretrieve(url, loc, reporthook=hook)
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

    target = 'mingw-w64' # only supported platform for now
    for library in sorted(DEPENDENT_LIBS, key=lambda x: DEPENDENT_LIBS[x]['order']):
        if target not in DEPENDENT_LIBS[library]['target']:
            print '%s: skipping (not available)' % library
            continue

        cfg = DEPENDENT_LIBS[library]['target'][target]
        built = True
        for path in cfg['result']:
            built = built and os.path.exists(join_path(dest, path))
        if built:
            print '%s: skipping (already built)' % library
            continue
        print '-' * 60, library
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
