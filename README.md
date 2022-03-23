# tp-2021-2c-Sinergia

***Atención***: El orden de los factores altera el producto, no se puede correr Kernel sin correr Swamp y Memoria primero, inicializar en el orden siguiente:

Iniciar Swamp
```
cd /home/utnso/tp-2021-2c-Sinergia/sharedlib/
make
cd /home/utnso/tp-2021-2c-Sinergia/matelib/
make
cd /home/utnso/tp-2021-2c-Sinergia/swamp/
make
cd /home/utnso/tp-2021-2c-Sinergia/swamp/bin
./swamp
```
Iniciar Memoria
```
cd /home/utnso/tp-2021-2c-Sinergia/sharedlib/
make
cd /home/utnso/tp-2021-2c-Sinergia/matelib/
make
cd /home/utnso/tp-2021-2c-Sinergia/memoria/
make
cd /home/utnso/tp-2021-2c-Sinergia/memoria/bin
./memoria
```
Iniciar Kernel
```
cd /home/utnso/tp-2021-2c-Sinergia/sharedlib/
make
cd /home/utnso/tp-2021-2c-Sinergia/matelib/
make
cd /home/utnso/tp-2021-2c-Sinergia/kernel/
make
cd /home/utnso/tp-2021-2c-Sinergia/kernel/bin
./kernel
```

Iniciar CarpinchoTest
```
cd /home/utnso/tp-2021-2c-Sinergia/sharedlib/
make
cd /home/utnso/tp-2021-2c-Sinergia/matelib/
make
cd /home/utnso/tp-2021-2c-Sinergia/carpinchoTest
make clean
make
cd /home/utnso/tp-2021-2c-Sinergia/carpinchoTest/bin
./carpinchoTest
```
Pruebas (Se debe especificar la prueba a correr por parametro):
```
cd /home/utnso/tp-2021-2c-Sinergia/sharedlib/
make
cd /home/utnso/tp-2021-2c-Sinergia/matelib/
make
cd /home/utnso
git clone https://github.com/sisoputnfrba/carpinchos-pruebas.git
cp /home/utnso/tp-2021-2c-Sinergia/matelib/src/matelib.h /home/utnso/carpinchos-pruebas/lib
cd /home/utnso/tp-2021-2c-Sinergia
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2021-2c-Sinergia/matelib/Debug
export LIBRARY_PATH=$LIBRARY_PATH:/home/utnso/tp-2021-2c-Sinergia/matelib/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2021-2c-Sinergia/sharedlib/Debug
export LIBRARY_PATH=$LIBRARY_PATH:/home/utnso/tp-2021-2c-Sinergia/sharedlib/Debug
cd /home/utnso/carpinchos-pruebas
make
cd build/
./a
```
path -> "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config"
