#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>

#define FILA "/tp1_pedidos"

typedef struct {
    int n1, n2;
    char resposta[64];
} Pedido;

typedef struct {
    int soma;
} Resposta;

volatile sig_atomic_t parar = 0;

void sinal_handler(int s) {
    parar = 1;
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = sinal_handler;
    sigaction(SIGINT, &sa, NULL);

    mqd_t pedidos = mq_open(FILA, O_RDONLY);

    if (pedidos == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    while (!parar) {
        Pedido p;

        if (mq_receive(pedidos, (char *)&p,
                       sizeof(p), NULL) == -1) {
            if (errno == EINTR && parar) break;
            if (errno == EINTR) continue;
            perror("mq_receive");
            break;
        }

        Resposta r;
        r.soma = p.n1 + p.n2;

        sleep(1);

        mqd_t resposta = mq_open(p.resposta, O_WRONLY);

        if (resposta == (mqd_t)-1) {
            perror("mq_open resposta");
            continue;
        }

        if (mq_send(resposta, (char *)&r, sizeof(r), 0) == -1)
            perror("mq_send resposta");

        mq_close(resposta);

        printf("B: calculou %d + %d = %d\n",
               p.n1, p.n2, r.soma);
        fflush(stdout);
    }

    mq_close(pedidos);
    return 0;
}
