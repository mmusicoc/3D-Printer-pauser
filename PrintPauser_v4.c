// Improved version by Oscar Garcia Lorenz
// Enables cmd execution with pause height as inline argument

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if __unix__ 
	void clear() {
    		system("clear");
	}
#elif defined(_WIN32) || defined(WIN32) 
	void clear() {
		system("cls");
	}
#endif

void snippet(FILE *g, char height[]){
	fprintf(g, "\n; ####################################################################################################################################\n");
	fprintf(g, "M117 Print paused for user interaction ; Show message on LCD\n\n");
	
	fprintf(g, "G91 ; Use relative coordinates\n");
	fprintf(g, "G1 Z5 E-1 F2500 ; Move up and relieve nozzle pressure to prevent oozing\n");
	fprintf(g, "G90 ; Use absolute coordinates\n");
	fprintf(g, "G1 X0 Y140 ; Move bed to front to enhance piece manipulation\n");
	fprintf(g, "M84 ; Disable steppers for handling\n");
	fprintf(g, "M117 Click button to resume ; Show message on LCD\n");
	fprintf(g, "M0 ; Print paused, resumes with user interaction\n\n");
	
	fprintf(g, "G28 X Y ; Home XY axes to endstops to retrieve coordinate origin\n");
	fprintf(g, "G91 ; Use relative coordinates\n");
	fprintf(g, "G1 Z-5 E+3 F2500 ; Move back to layer and gain nozzle pressure to prevent underextrusion\n");
	fprintf(g, "M117 Clean the nozzle, then click ; Show message on LCD\n");
	fprintf(g, "M0 ; Print paused, resumes with user interaction\n\n");
	
	fprintf(g, "G90 ; Use absolute coordinates\n");
	fprintf(g, "G92 Z%s ; Restore Z absolute position reference\n", height);
	fprintf(g, "; ####################################################################################################################################\n");
}

void pauseSnippet(FILE *g, char height[]){
	FILE *snippetFile;
	if((snippetFile = fopen("Snippet.txt","r")) == NULL) {
		printf("Error al leer el archivo con snippet de pausa.\n");
		snippet(g, height);
		printf("Se ha incrustado el fragmento por defecto.\n");
	}
	else {
		char aux;
		int read;
		fprintf(g, "\n");
		do{
			read = fscanf(snippetFile, "%c", &aux);
			if (aux != '%') fprintf(g, "%c", aux);
			else fprintf(g, "%s", height);
		} while (read == 1);
		fclose(snippetFile);
		fprintf(g, "\n");
	}
}

int compareHeight(char aux[], float height){
	float faux = strtof(aux, NULL);
	if ((faux < height + 0.01) && (faux > height - 0.01)) return 1;
	return 0;
}

int foundZ(FILE *f, FILE *g, float height){
	char aux[8], pauseHeight[8];
	fscanf(f, "%s", aux);
	fprintf(g, "%s", aux);
	if ((aux[0] >= '0' && aux[0] <= '9') && compareHeight(aux, height)){
		strcpy(pauseHeight, aux);
		fscanf(f, " %s", aux);
		fprintf(g, " %s\n", aux);
		pauseSnippet(g, pauseHeight);
		return 1;	
	}
	return 0;
}

void pauseRoutine(FILE *f, FILE *g, float height){
	char aux;
	int read, done = 0;
	do{
		do {
			read = fscanf(f, "%c", &aux);	
			fprintf (g, "%c", aux);
		} while (aux != 'Z' && read == 1);
		if (read == 1) {
			if (foundZ(f, g, height) == 1) done = 1;
		}
	} while (read == 1);
	if (done == 1) printf("\nAltura Z = %.3f ha sido encontrada y se ha incluido una pausa al empezar dicha capa.\n\n", height);
	else printf("\nError: Altura Z = %.3f NO ha sido encontrada.\n\n", height);

}

void welcome(char input[], char output[]){
	int i=0, j=0;
	char extension[8];
	strcpy(output, input);
	do i++;
	while (output[i]!= '.');
	output[i]='\0'; 
	i++;
	do{
		extension[j]=output[i];
		i++; j++;
	} while (output[i]!='\0');
	strcat(output, "-paused.");
	strcat(output, extension);
	
	printf("\nEl archivo output se llamara \"%s\".\n\n", output);
}

int help(char* cmd) {
	printf("\nEste es un programa que le permite pausar la impresion 3D a cierta altura.\n");
	printf("A la altura indicada, incrustara el fragmento de gcode que se encuentre en \"Snippet.txt\".\n\n");
	printf("\nModo de empleo: %s [FICHERO] [ALTURA]", cmd);
	printf("\nFICHERO: Nombre exacto del archivo original");
    printf("\nALTURA: Altura a la que debe pausar en mm\n\n");
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		help(argv[0]);
		return 0;
	}

	char inputName[32], outputName[32];
	float height;	
    FILE *inputFile, *outputFile;
    char values;
	int action;

	strcpy(inputName, argv[1]);
	height = atof(argv[2]);
	
	struct stat buffer;   
  	if (stat(inputName, &buffer) != 0) {
		printf("\nError: El archivo %s no existe.\n\n", inputName);
		help(argv[0]);
		return 0;
	} else if (height == 0.0) {
		printf("\nError: Altura no valida.\n\n");
		help(argv[0]);
		return 0;
	}

	welcome(inputName, outputName);

    if((inputFile = fopen(inputName,"r")) == NULL) printf("\nError al abrir el archivo.\n");
    else if((outputFile = fopen(outputName,"w+")) == NULL) printf("\nError al generar el archivo.\n");
    else {
        pauseRoutine(inputFile, outputFile, height);
        fclose(outputFile);
        fclose(inputFile);
        printf("Accion realizada y nuevo archivo guardado con exito!\n");
	}
	return 0;
}
