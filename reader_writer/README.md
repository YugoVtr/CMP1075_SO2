Para executar:
gcc main.c -o main  
sh make.sh <Quantidade_escritor> <Quantidade_leitor>

São executados (Quantidade_escritor + Quantidade_leitor) vezes o programa "main". 
Cada processo compartilha memoria através do buffer e o controle de acesso e feito através de semáforo. 
