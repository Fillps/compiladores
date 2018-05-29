/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Santos
*/

#include <stdio.h>
#include <stdlib.h>

static FILE *cfp = NULL;

void code_init(const char *filename)
{
    //verificar se code_init já foi chamada
    if (cfp){
        fprintf (stderr, "%s:%d já foi chamada, abort()\n", __FUNCTION__, __LINE__);
        abort();
    }

    //se o nome do arquivo for válido, abre arquivo com esse nome para escrita
    if (filename){
        cfp = fopen (filename, "w");
        if (!cfp){
            fprintf (stderr, "%s:%d não conseguiu abrir o arquivo %s para escrita\n", __FUNCTION__, __LINE__, filename);
            abort();
        }
    }else{
        cfp = stdout;
    }
}

void code_close(){
    if (cfp){
        fclose(cfp);
    }
    else{
        fprintf (stderr, "%s:%i foi chamada, mas code_init não foi chamada antes, abort()\n", __FUNCTION__, __LINE__);
        abort();
    }
}

