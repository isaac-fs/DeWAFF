# Implementación y validación de un algoritmo de abstracción de imagen en C++
Repositorio dedicado a avances del proyecto elétctrico.<br>
**Estudiante**: Isaac F. Fonseca Segura.<br>
**Profesor guía**: Dr.rer.nat. Francisco Siles Canales.

# Pasos seguidos
Esta sección sirve como referencia para la metodología del trabajo escrito.
- Configurar una VM con Ubuntu 22.04 LTS
- Leer el material de [Open CV introduction](https://www.opencv-srf.com/p/introduction.html)
- Seguir la guía de instalación de [Instalation tutorial - Linux](https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html)
- Instalar VSCODE y las extensiones para C++
<br>

# Idea principal
## Clase DEWAFF
La clase DEWAFF debe contar con un constructor que inicialice los valores necesarios para iniciar el filtrado de una imagen.

### Métodos
La clase DEWAFF debe contar con métodos que realicen los siguientes pasos:

- Un método que permita cargar una imágen desde una ruta local. Adquirir el tamaño de la imagen y guardar otros parámetros que sean relevantes. Y finalmente definir el formato de la imágen (color, densidad de pixeles).

- Un método que permita escoger el tipo de filtrado del framework utilizar (es posible pasar un string como parámetro). Este debe llamar a las funciones de filtrado necesarias y pasarles los parámetros que estas necesiten. Una vez terminado debe guardar la imágen con sufijo, ej: "imagen_\<BF>_\<DEWAFF>.png" si se utiliza el filtro bilateral.

### Funciones
Se debe implementar una función por cada tipo de operación que el framework implemente. Por ejemplo, se deben implementar los filtros y las funciones que estos requieran.

## Pruebas
Con imágenes de formato FHD o HD se debe probar cada configuración del framework para asegurar el correcto funcionamiento del mismo. Se pretende usar imágenes en estas resoluciones para evaluar los tiempos de funcionamiento del framework y verificar visualmente los resultados. 

Como pruebas finales se planea procesar imágenes de gran tamaño obtenidas de microoscopios u otros dispositivos.
