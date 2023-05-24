#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

//Constantes
#define M 96000
#define A 5
#define B 7
#define p 96001
#define NUM_FUNC 7
#define tamano 32

//Redefinici—n de variables
typedef uint32_t BITS; //BITS es un tipo de dato unsigned int que
                        //contiene 32bits o 4 bytes.

//definimos un tipo de funci—n 'especial' para las hash, con un apuntador. Esto para agregarles un tipo de dato al insertarlas en la estructura del filtro bloom.
typedef BITS (*hashB_func)(const char *data);

//------------------------ Funciones hash --------------------------
BITS strAint(char *contra){

  BITS x = 0;
  int len = strlen(contra);

  for(int i = 0; i < len; i++){
    if(i%2==1){
      x += contra[i];
    }
    else{
      x += contra[i]*9;
      x *= contra[i];
    }
  }

  return x;
}

BITS hashUno(char *contra) {

  int a = A;
  int b = B;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashDos(char *contra) {

  int a = A*2;
  int b = B*3;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashTres(char *contra) {

  int a = A*3;
  int b = B*3;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashCuatro(char *contra) {

  int a = A*4;
  int b = B*4;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashCinco(char *contra) {

  int a = A*5;
  int b = B*5;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashSeis(char *contra) {

  int a = A*6;
  int b = B*6;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

BITS hashSiete(char *contra) {

  int a = A*7;
  int b = B*7;

  BITS x = strAint(contra);

  BITS hash = ((a*(x) + b)%p)%M;

  return hash;
}

//------------------------Arreglo de bits----------------------

//Funci—n que crea el arreglo de bits
BITS *newBarray(){

  //cantidad de pedazos de 32 bits de memoria  que ocupamos
  size_t memory_size = M/tamano;

  //Verifica si el tamano "M" del arreglo es mœltiplo de 32
  if(M % tamano != 0){
    memory_size += 1; //Agrega un bloque m‡s a la memoria alojada
  }

  //Reserva la memoria que acabamos de calcular e inicializa sus bits a 0
  BITS *barray = calloc(memory_size, sizeof(BITS));

  return barray; //regresa el arreglo de bits
}

//----------Operaciones a nivel bits para el arreglo de bits---------

//funci—n para saber si un bit est‡ en 0 o 1
bool obtenerBit(BITS *barray, BITS hash) {

  //pedazo de memoria en el que se encuentra el bit
  size_t chunk = hash / tamano;

  //extrae el pedazo de memoria de nuestro arreglo de bits y lo 'copea'
  //en una variable, pues lo necesitaremos shiftear para comparar y no
  //queremos afectar el arreglo de bits real
  BITS byte = barray[chunk];

  //posici—n del bit dentro del pedazo de memoria (offset)
  size_t bit = hash % tamano;

  //a 'byte' le quita (hace un shift de izq a der) la cantidad de bits
  //obtenidos en la variable 'bit', dejando as’ como primer bit al que
  //queremos comparar.
  //DespuŽs compara lo que nos qued— con 1 usando el operador '&' o
  //and para bits, el cual devuelve todos los bits que estŽn encendidos
  //tanto en 'byte' despuŽs del shift como en 1 (el cual œnicamente
  //tiene encendido el primer bit de der a izq).
  //con esto compara si el bit est‡ en 1 o en 0 y lo devuelve
  return (byte>>bit) & 1;
}

//funci—n para asignar a 1 un bit
void asignarBit(BITS *barray, BITS hash){

  //pedazo de memoria en el que se encuentra el bit
  size_t chunk = hash / tamano;

  //en este caso al extraer el pedazo de memoria de nuestro arreglo de
  //bits utilizamos un apuntador, pues en este caso no shifteamos este,
  //si no que solo comparamos entre dos chunks y copia al apuntador
  //todos los bits que estŽn como 1 en ambos chunks
  BITS *byte = &(barray[chunk]);

  //posici—n del bit dentro del pedazo de memoria (offset)
  size_t bit = hash % tamano;

  //asignamos a 1 el bit, para ello
  //toma un chunk del tamano de BITS y lo inicializa en 1, para
  //despuŽs shiftearlo (de der a izq) las veces que indique 'bit',       //dejando as’ en 1 el bit deseado.
  //despuŽs compara este chunk con el que apunta 'byte' mediante el
  //operador l—gico '|' o or, copeando as’ cualquier bit en 1 de
  //ambos chunks en el arreglo original
  *byte |= ((BITS)1) << bit;
}

//----------------------Creaci—n del filtro-----------------------

//estructura del filtro bloom
typedef struct{
  BITS *barray;
  hashB_func *hash_functions;
} filtro_bloom;

//funci—n creadora del filtro bloom
filtro_bloom *newBloom(size_t num_functions, ...){
  //'...' representan las n funciones hash

  //creamos una lista para las funciones hash que usaremos (...)
  va_list hashes; //es un macro

  //creamos el filtro y asignamos sus valores
  filtro_bloom *filter = malloc(sizeof(*filter));
  filter->barray = newBarray();
  filter->hash_functions = malloc(sizeof(hashB_func)*num_functions);

  //obtenemos nuestras funciones hash de los par‡metros y las metemos
  //en la lista
  va_start(hashes, num_functions); //macro

  for(int i=0; i < num_functions; i++){
    //con el macro pasaremos todas las funciones hash, con su tipo
    //de dato especial
    filter->hash_functions[i] = va_arg(hashes, hashB_func);
  }

  //terminamos de usar el macro
  va_end(hashes); //macro

  return filter;
}

//------------Funciones generales para manejar el filtro-----------

//funci—n para insertar un nuevo elemento en el filtro
void insertar(filtro_bloom *filter, const char *contra){

  BITS hash = 0;

  for(int i = 0; i < NUM_FUNC; i++){
    //aplicamos el hash y lo guardamos
    hash = filter->hash_functions[i](contra);
    asignarBit(filter->barray, hash);
  }
}

//funci—n para buscar algœn elemento en el filtro
bool buscar(filtro_bloom *filter, const char *contra){

  BITS hash = 0;

  for(int i = 0; i < NUM_FUNC; i++){
    //aplicamos el hash y lo buscamos
    hash = filter->hash_functions[i](contra);

    if(!obtenerBit(filter->barray, hash)){
      //si algœn hash est‡ en 0, entonces no est‡ en el arreglo
      return false;
    }
  }

  return true; //si sale del ciclo estamos casi seguros que s’ est‡
}

//----------------------Lectura de archivos--------------------------

//funci—n para leer archivo e insertar los datos del archivo en el filtro
void readInsert(filtro_bloom *filtro, char* archive){

  FILE *input = fopen(archive, "r"); //abre el archivo

  /*
  Los siguientes son par‡metros que recibir‡ la funci—n getline,
  con los cuales esta reservar‡ memoria din‡mica o un buffer para
  leer los datos que reciba. La misma funci—n los modificar‡ acorde
  a sus necesidades. Al final este buffer debe ser liberado
  */
  char *contra = NULL;//almacenar‡ cada contrase–a, una a la vez
  size_t contra_len = 0; //tamano de la contrase–a actual

  //ciclo que itera por cada l’nea del archivo txt
  while (getline(&contra, &contra_len, input) != -1){
    //inserta al filtro la contrase–a que est‡ leyendo
    insertar(filtro, contra);
  }

  fclose(input); //cierra el archivo
  free(contra); //libera la memoria din‡mica que us— el getline
}

//funci—n para leer un archivo y buscar sus datos en el filtro.
void readSearch(filtro_bloom *filtro, char* archive){

  FILE *input = fopen(archive, "r"); //abre el archivo

  char *contra = NULL;//almacenar‡ cada contrase–a, una a la vez
  size_t contra_len = 0; //tamano de la contrase–a actual

  //ciclo que itera por cada l’nea del archivo txt
  while (getline(&contra, &contra_len, input) != -1){
    //buscar en el filtro la contrase–a que est‡ leyendo
    if(buscar(filtro, contra)){
      printf("Probablemente ya est‡ en el filtro: %s", contra);
    }
    else{
      printf("Aœn no est‡ en el filtro: %s", contra);
    }
  }

  fclose(input); //cierra el archivo
  free(contra); //libera la memoria din‡mica que us— el getline
}

//funci—n para user input
void consoleConsulta(filtro_bloom *filtro){

  bool again = true;
  char *input = NULL;
  size_t input_len = 0;

  do{
    system("clear");

    printf("Ingrese contrase–a a buscar: ");
    getline(&input, &input_len, stdin);

    if(buscar(filtro, input)){
      printf("Es casi seguro que su contrase–a est‡ en el conjunto\n");
    }
    else{
      printf("Su contrase–a definitivamente no est‡ en el conjunto\n");
    }

    printf("\nÀDesea buscar un nuevo elemento?\n");
    getline(&input, &input_len, stdin);

    if(input[0] == 'N' || input[0] == 'n'){
      again = false;
    }

  }while(again);
}

//------------------------   PRUEBAS   --------------------------------

int helpert(filtro_bloom *filtro, char *archive2, char *contra){

  FILE *input2 = fopen(archive2, "r");

  char *contra2 = NULL;
  size_t contra_len2 = 0;


  int flag0 = 0;

  while(getline(&contra2, &contra_len2, input2) != -1){

      if(strlen(contra) == strlen(contra2)){

        flag0 = 0;

        for(int i = 0; i < strlen(contra); i++){
          if(contra[i] != contra2[i]){
              flag0 = 1;
              break;
          }
        }

        if(flag0 == 0){
          fclose(input2);
          free(contra2);
          return 1;
        }
      }
  }

  fclose(input2);
  free(contra2);

  return 0;
}

int helpert2(filtro_bloom *filtro, char *archive2, char *contra){
  FILE *input2 = fopen(archive2, "r");

  char *contra2 = NULL;
  size_t contra_len2 = 0;
  int flag2 = 0;

  while(getline(&contra2, &contra_len2, input2) != -1){
    if(contra == contra2){
      flag2 = 1;
      break;
    }
  }

  fclose(input2);
  free(contra2);

  if(flag2==0){
    return 1;
  }
  else{
    return 0;
  }
}

//usado para probar el filtro con un archivo
void tester(filtro_bloom *filtro, char *archive, char *archive2){

  printf("\n%s\n", archive);
  FILE *input = fopen(archive, "r"); //abre el archivo

  char *contra = NULL;//almacenar‡ cada contrase–a, una a la vez
  size_t contra_len = 0; //tamano de la contrase–a actual

  float tp = 0; //true positives
  float fp = 0; //false positives
  float tn = 0; //true negatives


  //ciclo que itera por cada l’nea del archivo a comprobar
  while(getline(&contra, &contra_len, input) != -1){

    //buscar en el filtro la contrase–a que est‡ leyendo
    if(buscar(filtro, contra)){

      if(helpert(filtro, archive2, contra) > 0){
        tp++;
      }
      else{
        fp++;
      }
    }
    else{
      if(helpert2(filtro, archive2, contra) == 1){
        tn++;
      }
    }
  }

  printf("\nCantidad de positivos: %f", tp+fp);
  printf("\nCantidad de true positives: %f", tp);
  printf("\nCantidad de false positives: %f", fp);
  printf("\nCantidad de true negatives: %f", tn);
  printf("\nCantidad de false negatives: %f", 200-(tp+fp+tn));
  printf("\nPrecision: %f", tp/(tp+fp));
  printf("\nRecall: %f", tp/120);

  fclose(input); //cierra el archivo
  free(contra); //libera la memoria que us— el getline
}

//----------------------
//-------- MAIN --------
//----------------------
int main(int argc, char *argv[]) {

  //creamos filtro
  filtro_bloom *filtro = newBloom(NUM_FUNC, hashUno, hashDos, hashTres, hashCuatro, hashCinco, hashSeis, hashSiete);


  //leemos el archivo e insertamos sus datos en el filtro
  readInsert(filtro, "/Users/brami/Downloads/dump/datos_contras_insertar.txt");

  //funci—n para bœsquedas por user input
  consoleConsulta(filtro);

  //Funci—n para las pruebas
  //tester(filtro, "/Users/brami/Downloads/Pruebas/Prueba31.txt", "/Users/brami/Downloads/dump/datos_contras_insertar.txt");

  return 0;
}
