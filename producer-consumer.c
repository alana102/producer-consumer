/* 
Integrantes da equipe: 
Alana Maria Sousa Augusto
Clara Cruz Alves 
Jennifer Marques de Brito
*/

// item G da 1ª questão
// 6 threads produtoras e 2 consumidoras utilizando mutex
// Alana Maria Sousa Augusto

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define NUM_PRODUCERS 6 // nº máximo de produtores
#define NUM_CONSUMERS 2 // nº máximo de consumidores

int count = 0; // nº de elementos no buffer
int buffer[5]; // buffer com 5 espaços
int produtores = NUM_PRODUCERS; // quantos produtores estão disponíveis

// garante a exclusão mútua através de um mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
// variável que avisa quando o buffer está cheio (evita condições de corrida)
pthread_cond_t buffer_cheio = PTHREAD_COND_INITIALIZER;
// variável que avisa quando o buffer não está cheio (evita condições de corrida)
pthread_cond_t buffer_nao_cheio = PTHREAD_COND_INITIALIZER;

void* producer (void* args){
    int n = *((int *) args); // nº de iterações para a thread
    pthread_t tid = pthread_self(); // id da thread

    while(n-- > 0){
        // escolhe um valor aleatório entre 1 e 1000
        int valor = rand()% 1000+1; 
        // espera entre 1 e 5 segundos
        sleep(rand() % 5 + 1);

        // entra na região crítica
        pthread_mutex_lock(&mutex);

        // enquanto meu buffer estiver cheio
        while(count == 5){
            pthread_cond_wait(&buffer_nao_cheio, &mutex);
        }

        // insere um valor no buffer
        buffer[count++] = valor;
        // printa o que está ocorrendo na thread
        printf("(P) TID: %lu | VALOR: R$ %d | ITERACAO: %d\n", tid, valor, n + 1);
        
        // se o buffer encheu
        if (count == 5){
            // consumidor é acordado
            pthread_cond_signal(&buffer_cheio);
        }

        // sai da região crítica
        pthread_mutex_unlock(&mutex);

    }

    pthread_mutex_lock(&mutex);
    produtores--; // diminui a quantidade de produtores disponíveis
    pthread_cond_broadcast(&buffer_cheio); // se tiver consumidor esperando, acorda ele
    pthread_mutex_unlock(&mutex);

    // avisa que a thread terminou
    printf("(P) TID: %lu finalizou\n", tid);

}

void *consumer(void *args){
    int it = 0;
    pthread_t tid = pthread_self(); // id da thread

    while(produtores != 0){
        // entra na região crítica
        pthread_mutex_lock(&mutex);

        // espera o buffer encher
        while(count < 5 && produtores > 0) {
            pthread_cond_wait(&buffer_cheio, &mutex);
        }

        // caso não tenha mais produtores disponíveis
        if(count < 5 && produtores == 0){
            pthread_mutex_unlock(&mutex);
            break;
        }

        int dados[5];

        // armazena os dados do buffer
        for(int i = 0; i < 5; i++){
            dados[i] = buffer[i];
        }

        // esvazia o buffer
        count = 0;

        // acorda produtores
        pthread_cond_broadcast(&buffer_nao_cheio);
        // sai da região crítica
        pthread_mutex_unlock(&mutex);

        int soma = 0;

        for(int i = 0; i < 5; i++){
            soma += dados[i]; // soma os valores
        }

        int media = soma/5; // calcula a media

        it++; // incrementa o contador de iteração

        // printa informações sobre o momento da execução da thread
        printf("(C) TID: %lu | MEDIA: R$ %d | ITERACAO: %d\n", tid, media, it );
    }

    // avisa que a thread terminou
    printf("(C) TID: %lu finalizou\n", tid);
}



int main(void){

    srand(time(NULL)); // gerador de números

    pthread_t produtores[NUM_PRODUCERS];
    pthread_t consumidores[NUM_CONSUMERS];
    int iteracoes[NUM_PRODUCERS];

    // criação de produtores com nº de iterações entre 20 e 30
    for(int i = 0; i < NUM_PRODUCERS; i++){
        iteracoes[i] = 20 + rand() % 11;
        pthread_create(&produtores[i], NULL, producer, &iteracoes[i]);
    }

    // criação de consumidores
    for(int i = 0; i < NUM_CONSUMERS; i++){
        pthread_create(&consumidores[i], NULL, consumer, NULL);
    }

    // aguarda as produtoras finalizarem
    for(int i = 0; i < NUM_PRODUCERS; i++){
        pthread_join(produtores[i], NULL);
    }

    // aguarda as consumidoras finalizarem
    for(int i = 0; i< NUM_CONSUMERS; i++){
        pthread_join(consumidores[i], NULL);
    }

}
