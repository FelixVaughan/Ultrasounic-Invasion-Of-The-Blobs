from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

ext_modules = [
    Extension( "us100Mod",
               sources=["us100.pyx", 'ultra.c'],
               include_dirs = ["/opt/vc/include"],
               libraries = [ "bcm_host" ],
               library_dirs = ["/opt/vc/lib"]
    )
]


    
setup(
    name = "py_us100",
    ext_modules = cythonize( ext_modules )
)

