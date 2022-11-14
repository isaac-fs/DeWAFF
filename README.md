# Implementación y validación de un algoritmo de abstracción de imagen en C++
Repositorio dedicado a avances del proyecto elétctrico.<br>
**Estudiante**: Isaac F. Fonseca Segura.<br>
**Profesor guía**: Dr.rer.nat. Francisco Siles Canales.

## Tabla de contenidos
- [Implementación y validación de un algoritmo de abstracción de imagen en C++](#implementación-y-validación-de-un-algoritmo-de-abstracción-de-imagen-en-c)
  - [Tabla de contenidos](#tabla-de-contenidos)
  - [Pasos seguidos](#pasos-seguidos)
  - [Implementación actual](#implementación-actual)
  - [Clases](#clases)
  - [Generación de la documentación](#generación-de-la-documentación)
  - [Compilación del proyecto](#compilación-del-proyecto)
  - [Ejecución del framework](#ejecución-del-framework)
    - [Procesar imagen](#procesar-imagen)
    - [Procesar imagen y hacer benchmark](#procesar-imagen-y-hacer-benchmark)
    - [Procesar vídeo](#procesar-vídeo)
    - [Procesar vídeo y hacer benchmark](#procesar-vídeo-y-hacer-benchmark)
  - [Idea inicial de implementación](#idea-inicial-de-implementación)
    - [Clase DEWAFF](#clase-dewaff)
    - [Métodos](#métodos)
    - [Funciones](#funciones)
    - [Pruebas](#pruebas)

## Pasos seguidos
Esta sección sirve como referencia para la metodología del trabajo escrito.
- Configurar una VM con Ubuntu 22.04 LTS
- Leer el material de [Open CV introduction](https://www.opencv-srf.com/p/introduction.html)
- Seguir la guía de instalación [OpenCV Instalation tutorial - Linux](https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html)
- Instalar VSCode y las extensiones necesarias para C++. Además de extensiones para soporte de CMake y Doxygen
- Investigar sobre el framework DeWAFF
- Comenzar implementación en C++
- Hacer un fork the [ParallelDeWAFF](https://github.com/david-prado/ParallelDeWAFF) por David Prado (ITC)
- Automatización de compilación con CMake
- Revisión extensiva y corrección de inconsistencias en el fork
- Pruebas con imágenes y revisión de resultados
- Pruebas con vídeo y revisión de resultados
- Documentación del código con Doxygen
- Elaboración del trabajo escrito
<br>

## Implementación actual
En la implementación actual se cuenta no sólo con la clase DeWAFF, pero también con clases que asisten al procesado de la imagen como la clase NonAdaptiveUSM. Además se incluyen clases que se encargan del preprocesado de la imagen y la presentación del programa en la terminal. Finalmente hay clases con métodos de asistencia como Timer y Tools.

## Clases
Las clases implementadas y/o adaptadas y sus métodos son las siguientes:
- DeWAFF
    - DeceivedBilateralFilter
    - NonAdaptiveUSMFilter
    - LaplacianKernel
    - GaussianKernel
    - GaussianExponentialFactor
- FileProcessor
    - processImage
    - processVideo
    - processFrame
    - errorExit
- CLI
    - run
    - help
- Timer
    - start
    - stop
- Tools
    - meshGrid
    - getMinMax

Estas clases y funciones se encuentran extensivamente documentadas en formato Doxygen. Para generar la documentación se deben seguir los siguientes pasos

## Generación de la documentación
Se deben introducir los siguientes comandos
```bash
    cd doxygen/
    doxygen Doxyfile
```
Después se debe abrir el enlace símbolico en el directorio raíz `DeWAFF-B52786/` del proyecto llamado `index.html`. Esto abrirá en su navegador la documentacion del proyecto. Doxygen debe estar instalado previamente.

## Compilación del proyecto
La compliación de este proyecto se realiza de manera automatizada con `cmake` y `make`. Para crear un ejecutable debe seguir los siguientes pasos en el directorio raíz `DeWAFF-B52786/`. Todas las dependencias de `OpenCV` deben estar instaladas previamente.
```bash
    cmake .
    cmake --build .
```
Si hace cambios al código podrá actualizar el proyecto con `make`.

## Ejecución del framework
Para correr el programa se debe haber generado el binario `DeWAFF` en el directorio raíz al haber seguido los pasos de compilación. Una vez con este binario se pueden procesar imágenes y vídeo con la opción de hacer un benchmark.

Para correr el programa use el siguiente comando en el directorio raíz `DeWAFF-B52786/`, o donde desee ubicar el programa

### Procesar imagen
```bash
    ./DeWAFF -i <ruta del archivo>
```

### Procesar imagen y hacer benchmark
```bash
    ./DeWAFF -i <ruta del archivo> -b <número de iteraciones>
```

### Procesar vídeo
```bash
    ./DeWAFF -v <ruta del archivo>
```

### Procesar vídeo y hacer benchmark
```bash
    ./DeWAFF -v <ruta del archivo> -b <número de iteraciones>
```

El resultado se generará en la `ruta del archivo` escogido y se le agregará el sufijo DeWAFF de forma que el resultado se mostrará de la forma `<nombre_del_archivo>_DeWAFF.<ext>`. En el directorio raíz se encuentran un par de ejemplos en el directorio `img/`.

A los vídeos se les agrega la extensión `.avi` y las imágenes la extensión `.png` por defecto.

## Idea inicial de implementación
La idea original era crear desde cero una implementación de DeWAFF. Más adelante se esperaba partir de una implementación del algoritmo en Matlab, lamentablemente esta se perdió, por lo que se comenzó a implementar el framework desde cero. A medio camino se dio con una implementación de código abierto. Se hizo un fork de esta y se comenzó a trabajar con esto como nueva base. A pesar de tener ciertas inconsistencias tenía las bases necesarias y una implementación del filtro bilateral funcional.

A continuación se muestran las ideas originales del proyecto, las cuales terminaron siendo consistentes con la implementación actual.
### Clase DEWAFF
La clase DEWAFF debe contar con un constructor que inicialice los valores necesarios para iniciar el filtrado de una imagen.

### Métodos
La clase DEWAFF debe contar con métodos que realicen los siguientes pasos:

- Un método que permita cargar una imágen desde una ruta local. Adquirir el tamaño de la imagen y guardar otros parámetros que sean relevantes. Y finalmente definir el formato de la imágen (color, densidad de pixeles).

- Un método que permita escoger el tipo de filtrado del framework utilizar (es posible pasar un string como parámetro). Este debe llamar a las funciones de filtrado necesarias y pasarles los parámetros que estas necesiten. Una vez terminado debe guardar la imágen con sufijo, ej: "imagen_\<BF>_\<DEWAFF>.png" si se utiliza el filtro bilateral.

### Funciones
Se debe implementar una función por cada tipo de operación que el framework implemente. Por ejemplo, se deben implementar los filtros y las funciones que estos requieran.

### Pruebas
Con imágenes de formato FHD o HD se debe probar cada configuración del framework para asegurar el correcto funcionamiento del mismo. Se pretende usar imágenes en estas resoluciones para evaluar los tiempos de funcionamiento del framework y verificar visualmente los resultados.

Como pruebas finales se planea procesar imágenes de gran tamaño obtenidas de microoscopios u otros dispositivos.