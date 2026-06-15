# CGII-Tarea1-30182893-Jhonatan Homsany

## Índice

1. [Introducción](#introducción)
2. [Guía de compilación](#guía-de-compilación)
3. [Librerías utilizadas](#librerías-utilizadas)
4. [Requerimientos implementados](#requerimientos-implementados)
5. [Guía de uso](#guía-de-uso)

   5.1 [Controles](#controles)

   5.2 [Interfaz Gráfica de Usuario (GUI)](#interfaz-gráfica-de-usuario-gui)

6. [Buenas prácticas llevadas a cabo en el desarrollo](#buenas-prácticas-llevadas-a-cabo-en-el-desarrollo)
7. [Dificultades encontradas](#dificultades-encontradas)
8. [Funcionamiento del programa](#funcionamiento-del-programa)

---

## Introducción.

El presente documento constituye la guía técnica y operativa del sistema desarrollado para la cátedra de Tópicos Avanzados en Computación Gráfica en la Escuela de Computación de la Facultad de Ciencias de la Universidad Central de Venezuela.

El proyecto consiste en un renderizador de escenas que permite explorar las diferentes técnicas impartidas en la materia como iluminación, renderización por medio de rasterización o raytracing, texturas, diversas técnicas para el sombreado de objetos, manipulación de escenas y los atributos de material que puede tener un objeto.

## Guía de compilación.

Este proyecto fue realizado y probado utilizando Visual Studio 2026. Para poder ejecutarlo, es necesario tener instalado MSVC - v143 - VS 2022 C++ x64/x86 build tools (v14.44 - 17.14) como herramienta de construcción de proyectos para así poder compilarlo y ejecutarlo correctamente.

Adicionalmente, la configuración de ejecución se ha establecido en modo $Release$ para plataformas $x64$.

Una vez establecidas las configuraciones, basta con utilizar el depurador local de Windows en Visual Studio 2022 para ejecutar el proyecto.

Este proyecto también se encuentra disponible en la plataforma GitHub a través del siguiente enlace: https://github.com/JhonatanJHZ/CGII-Tarea1-30182893-JhonatanHomsany

## Librerías utilizadas.

Ante la ausencia de un manejador de paquetes para programas hechos en C++, el proyecto fue realizado utilizando la configuración base de CMake definida por Visual Studio. Luego, a partir de CMake, se instalaron las siguientes librerías necesarias para la realización del proyecto:

- Glad.
- GLFW.
- GLM.
- ImGUI.
- TinyGLTF.

Adicionalmente, fue incluída la implementación de TinyFileDialog para acudir al explorador de archivos del sistema operativo del computador a la hora de ubicar las rutas de los archivos GLB/GLTF que se usarán en la escena. De esta manera, se evitó que el usuario deba escribir manualmente la ruta donde está o donde quiere guardar su archivo.

TinyFileDialogs es una librería de código abierto desarrollada por el usuario Guillaume Vareille y fue publicada en el 2017. Se puede encontrar información adicional de la librería en la dirección https://github.com/native-toolkit/libtinyfiledialogs/blob/master/tinyfiledialogs.c

## Requerimientos implementados.

El presente proyecto contiene la implementación completa de los requerimientos obligatorios solicitados en el documento enviado por el grupo docente. Adicionalmente, se implementó el requisito opcional A, que describe lo siguiente:

- Shadow volumes en una escena controlada (preferiblemente usando Stencil Buffer). Se debe poder visualizar el mallado del volumen de sombras y su intersección con los objetos que serán cubiertos por la sombra.

## Guía de uso.

El programa cuenta con los controles básicos en la interfaz de ImGUI para ayudar al usuario a controlar el programa en tiempo de ejecución sin tener que cambiar entre la pestaña de esta documentación y la aplicación. Sin embargo, a continuación se describen en mayor detalle las funcionalidades de la aplicación.

### Controles.

Los movimientos de cámara se implementaron tomando gran inspiración en el manejo de cámara en videojuegos de estilo first-person shooter (FPS) o Minecraft (en modo creativo). A pesar de no tener colisiones, se tomó inspiración para utilizar aspectos como el uso de WASD para el movimiento en el plano horizontal, SPACE para subir, LSHIFT para bajar.

Para controlar la dirección a la que apunta la cámara, el usuario puede mantener presionado el click derecho o la tecla Z mientras mueve su mouse para así ver a su alrededor.

Para aumentar o disminuir el field of view del programa (FOV), es posible utilizar la rueda del mouse.

Esto describe los controles utilizados con entradas de teclado y ratón. A continuación, veamos los controles asociados a la interfaz gráfica de usuario (GUI).

### Interfaz Gráfica de Usuario (GUI).

En la GUI, el usuario cuenta con los siguientes controles:

- Generación de objetos (pirámides, esferas, cubos y cilíndros).

- Agregar nuevas cámaras en la escena. Es importante mencionar que se otorgó libertad de movimiento a las cámaras generadas para que el usuario pueda ubicarlas en el lugar de su preferencia en caso de que quiera mantenerlas como cámaras de seguridad. Las cámaras guardan los atributos que tenían la última vez que fueron utilizadas.

- Cambiar el modo de renderizado de las cámaras.

- Modificar las propiedades de la cámara actual (Posición, target y FOV).

- Tras seleccionar un objeto usando Ray Picking, se podrán modificar sus atributos (posición, rotación, escala y color) y sus propiedades de PBR. Asimismo, el usuario tiene la opción de eliminar el objeto seleccionado de la escena.

- El usuario cuenta con los elementos para modificar los atributos del rayo láser en la escena que modifica el color del objeto que impacta. Puede modificar su origen, su dirección, color y t-min/t-max.

- Iluminación: Tipo de iluminación (Flat, Lambert, Phong y Blinn-Phong), posición de la luz, intensidad de la luz y luz ambiental. Además, al incluir PBR, ocurrió que la escena se volvió un poco más oscura. Por ende, se implementó un control de la exposición al PBR que permitiera manejar que tanta luz absorben los objetos y así tener mayor control sobre la iluminación de la escena.

- Para las sombras, se implementaron sombras planares, shadow mapping y shadow volumes. En shadow mapping, el usuario puede seleccionar el tipo de sombreado que desea, el bias para mitigar el shadow acné, la opción de solamente ver sombras, ver el mapa de profundidad, el uso de un bias adaptativo para mitigar el Peter Panning y un deslizador para configurar el suavizado de bordes con PCF. Por otro lado, en el modo shadow volumes, el usuario puede activar la renderización del mallado de los volúmenes utilizados para las sombras.

Finalmente, la GUI cuenta con los botones para:

- Importar archivo GLB/GLTF.
- Guardar y cargar escenas.
- Volver a generar la escena de la caja renderizada apenas se abre el programa.

## Buenas prácticas llevadas a cabo en el desarrollo.

El presente proyecto basa su implementación en el uso de buenas prácticas de la Ingeniería de Software. A pesar de tener un lenguaje que puede ser restrictivo para ciertas cosas, el diseño del algoritmo está basado en el principio SOLID. Particularmente, el desarrollo hizo énfasis en aprovechar el principio de responsabilidad única, el principio open/closed y la inyección de dependencias para lograr un código escalable y con una mantenibilidad sencilla. El uso de estos principios es posible gracias a la programación orientada a objetos, donde se aprovecha el concepto de polimorfismo y encapsulación.

## Dificultades encontradas.

La depuración y pruebas de este proyecto fueron llevadas principalmente en una escena generada manualmente y no a partir de la carga de escenas GLB/GLTF. Esto se debió a principalmente dos factores:

1. Ocurrieron problemas intentando generar escenas simples con motores 3D como Blender debido a fallos del programa.

2. Resultó complejo conseguir escenas en internet que tuvieran los elementos necesarios para probar las funcionalidades requeridas.

Sin embargo, la carga de archivos GLB/GLTF fue implementada y una escena puede ser cargada sin problemas. Asimismo, es posible construir una escena cargando múltiples archivos GLB/GLTF.

Por otro lado, en el modo raytracing se presentaron problemas de rendimiento cuando se intenta cargar un modelo que es computacionalmente complejo debido a que las colisiones de los rayos son manejadas verificando triangulo por triangulo su colisión. En consecuencia, mientras mayor sea la complejidad de un modelo tridimensional cargado, peor será el rendimiento del programa.

Finalmente, al implementar el PBR ocurre que, en el modo raytracing, se deben asignar ciertos valores al material para lograr un efecto de espejo en los objetos (el valor de metallic debe ser alto mientras que el rougness y ambient occlusion deben ser cercanos a 0). Sin embargo, al tener esta configuración en los objetos, el objeto puede mostrarse completamente negro debido al valor del ambient occlusion.

## Funcionamiento del programa.

<img width="1918" height="1137" alt="image" src="https://github.com/user-attachments/assets/3cd984c5-daf7-461f-9fd7-9157abf37146" />

<p align="center">
  <i><b>Fig. 1.</b> Escena generada manualmente y renderizada con rasterización.</i>
</p>

<img width="1918" height="1133" alt="image" src="https://github.com/user-attachments/assets/bfafc5d8-622d-4770-b5c1-8ca50d3bd668" />

<p align="center">
  <i><b>Fig. 2.</b> Escena generada manualmente y renderizada con Ray Tracing.</i>
</p>
