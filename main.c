#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    int codigo;
    char titulo[100];
    char autor[100];
    int ano;
    int emprestado; //Alugado = 1 e Disponível = 0
}Livro;


//Inicializando o banco

void inicializandoBanco(sqlite3 **db){
     char *erroMsg = 0;

    // Abrir (ou criar) o banco de dados
    if (sqlite3_open("biblioteca.db", db) != SQLITE_OK) {
        printf("Erro ao abrir o banco de dados.\n");
        exit(1);
    }

    // Criar a tabela 'livros' se não existir
    const char *sql =
        "CREATE TABLE IF NOT EXISTS livros ("
        "codigo INTEGER PRIMARY KEY,"
        "titulo TEXT NOT NULL,"
        "autor TEXT NOT NULL,"
        "ano INTEGER NOT NULL,"
        "emprestado INTEGER NOT NULL);";

    if (sqlite3_exec(*db, sql, 0, 0, &erroMsg) != SQLITE_OK) {
        printf("Erro ao criar a tabela: %s\n", erroMsg);
        sqlite3_free(erroMsg);
        sqlite3_close(*db);
        exit(1);
    }

    const char *sqlLeitores = "CREATE TABLE IF NOT EXISTS leitores ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "nome TEXT NOT NULL, "
                          "email TEXT NOT NULL UNIQUE, "
                          "telefone TEXT);";

if (sqlite3_exec(*db, sqlLeitores, NULL, NULL, NULL) != SQLITE_OK) {
    printf("Erro ao criar tabela leitores: %s\n", sqlite3_errmsg(*db));
}

}

//Cadastra o livro na base de dados
void cadastrarLivro(sqlite3 *db){
    char *erroMsg = 0;
    char sql[512];
    int codigo, ano;
    char titulo[100], autor[100];
    int emprestado = 0; //Livro não emprestado por padrão


    //Receber informações do livro

    printf("Digite o código único do livro:");
    scanf("%d", &codigo);

    fflush(stdin);

    printf("Digite o ano do livro:");
    scanf("%d", &ano);

    fflush(stdin);

    printf("Digite o nome do livro:");
    scanf("%s", &titulo);
    //fgets(titulo, sizeof(titulo), stdin);
    strtok(titulo, "\n");

    fflush(stdin);

    printf("Digite o autor do livro: ");
    scanf("%s", &autor);
    //fgets(autor, sizeof(autor), stdin);
    strtok(autor, "\n");

    fflush(stdin);


    //Comando para inserir os dados
    snprintf(sql, sizeof(sql),
        "INSERT INTO livros (codigo, titulo, autor, ano, emprestado) "
             "VALUES (%d, '%s', '%s', %d, %d);",
             codigo, titulo, autor, ano, emprestado);


    //Executando o comando

    if (sqlite3_exec(db, sql, 0, 0, &erroMsg) != SQLITE_OK) {
        printf("Erro ao cadastrar o livro: %s\n", erroMsg);
        sqlite3_free(erroMsg);
    } else {
        printf("Livro cadastrado com sucesso!\n");
    }

}

void buscarLivro(sqlite3 *db){
    int opcao, codigo;
    char titulo[100], sql[512];
    sqlite3_stmt *stmt;

    printf("\nOpções de busca:\n");
    printf("1. Buscar pelo código do livro\n");
    printf("2. Buscar pelo título do livro\n");
    printf("Escolha uma opção: ");
    scanf("%d", &opcao);
    getchar();

    if (opcao == 1) {
        // Busca pelo código do livro
        printf("Digite o código do livro: ");
        scanf("%d", &codigo);

        snprintf(sql, sizeof(sql),
                 "SELECT codigo, titulo, autor, ano, emprestado FROM livros WHERE codigo = %d;",
                 codigo);
    } else if (opcao == 2) {
        // Busca pelo título do livro
        printf("Digite o título do livro (ou parte dele): ");
        fgets(titulo, sizeof(titulo), stdin);
        strtok(titulo, "\n");

        snprintf(sql, sizeof(sql),
                 "SELECT codigo, titulo, autor, ano, emprestado FROM livros WHERE titulo LIKE '%%%s%%';",
                 titulo);
    } else {
        printf("Opção inválida.\n");
        return;
    }

    //Query SQL

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Executar a consulta e processar os resultados
    int encontrou = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        encontrou = 1;
        int codigo = sqlite3_column_int(stmt, 0);
        const unsigned char *titulo = sqlite3_column_text(stmt, 1);
        const unsigned char *autor = sqlite3_column_text(stmt, 2);
        int ano = sqlite3_column_int(stmt, 3);
        int emprestado = sqlite3_column_int(stmt, 4);

        printf("\n--- Livro Encontrado ---\n");
        printf("Código: %d\n", codigo);
        printf("Título: %s\n", titulo);
        printf("Autor: %s\n", autor);
        printf("Ano: %d\n", ano);
        printf("Status: %s\n", emprestado ? "Emprestado" : "Disponível");
    }

    if (!encontrou) {
        printf("\nNenhum livro encontrado.\n");
    }

    // Finaliza a query
    sqlite3_finalize(stmt);

}


void listarLivros(sqlite3 *db) {
    const char *sql = "SELECT codigo, titulo, autor, ano, emprestado FROM livros;";
    sqlite3_stmt *stmt;

    // Query SQL
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- Lista de Livros ---\n");

    int encontrou = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        encontrou = 1;

        int codigo = sqlite3_column_int(stmt, 0);
        const unsigned char *titulo = sqlite3_column_text(stmt, 1);
        const unsigned char *autor = sqlite3_column_text(stmt, 2);
        int ano = sqlite3_column_int(stmt, 3);
        int emprestado = sqlite3_column_int(stmt, 4);

        printf("\nCódigo: %d\n", codigo);
        printf("Título: %s\n", titulo);
        printf("Autor: %s\n", autor);
        printf("Ano: %d\n", ano);
        printf("Status: %s\n", emprestado ? "Emprestado" : "Disponível");
    }

    if (!encontrou) {
        printf("\nNenhum livro cadastrado.\n");
    }

    // Finaliza a query
    sqlite3_finalize(stmt);
}

void emprestarLivro(sqlite3 *db) {
    int codigo;
    char sql[256];
    sqlite3_stmt *stmt;

    printf("\n--- Emprestar Livro ---\n");
    printf("Digite o código do livro: ");
    scanf("%d", &codigo);

    // Verifica se o livro está disponível
    snprintf(sql, sizeof(sql), "SELECT emprestado FROM livros WHERE codigo = %d;", codigo);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int emprestado = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (emprestado) {
            printf("O livro já está emprestado.\n");
            return;
        }

        // Atualiza o status do livro para emprestado
        snprintf(sql, sizeof(sql), "UPDATE livros SET emprestado = 1 WHERE codigo = %d;", codigo);
        if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
            printf("Erro ao atualizar o status do livro: %s\n", sqlite3_errmsg(db));
        } else {
            printf("O livro foi emprestado com sucesso.\n");
        }
    } else {
        printf("Livro não encontrado.\n");
    }

    sqlite3_finalize(stmt);
}

void devolverLivro(sqlite3 *db) {
    int codigo;
    char sql[256];
    sqlite3_stmt *stmt;

    printf("\n--- Devolver Livro ---\n");
    printf("Digite o código do livro: ");
    scanf("%d", &codigo);

    // Verifica se o livro está emprestado
    snprintf(sql, sizeof(sql), "SELECT emprestado FROM livros WHERE codigo = %d;", codigo);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Erro ao preparar a consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int emprestado = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (!emprestado) {
            printf("O livro já está disponível.\n");
            return;
        }

        // Atualiza o status do livro para disponível
        snprintf(sql, sizeof(sql), "UPDATE livros SET emprestado = 0 WHERE codigo = %d;", codigo);
        if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
            printf("Erro ao atualizar o status do livro: %s\n", sqlite3_errmsg(db));
        } else {
            printf("O livro foi devolvido com sucesso.\n");
        }
    } else {
        printf("Livro não encontrado.\n");
    }

    sqlite3_finalize(stmt);
}

void cadastrarLeitor(sqlite3 *db) {
    char nome[100], email[100], telefone[15];
    char sql[256];

    printf("\n--- Cadastrar Leitor ---\n");

    printf("Digite o nome do leitor: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = '\0'; // Remover a quebra de linha

    printf("Digite o e-mail do leitor: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = '\0'; // Remover a quebra de linha

    printf("Digite o telefone do leitor (opcional): ");
    fgets(telefone, sizeof(telefone), stdin);
    telefone[strcspn(telefone, "\n")] = '\0'; // Remover a quebra de linha

    // Comando SQL para inserir o leitor no banco de dados
    snprintf(sql, sizeof(sql),
             "INSERT INTO leitores (nome, email, telefone) VALUES ('%s', '%s', '%s');",
             nome, email, telefone);

    // Executar a consulta
    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        printf("Erro ao cadastrar leitor: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Leitor cadastrado com sucesso!\n");
    }
}

int main(int argc, char *argv[]) {
    sqlite3 *db;
    int opcao;

    // Inicializa o banco de dados
    inicializandoBanco(&db);

        // Menu de opções
        printf("\n--- Sistema de Biblioteca ---\n");
        printf("1. Cadastrar Livro\n");
        printf("2. Buscar Livro\n");
        printf("3. Listar Livros\n");
        printf("4. Emprestar Livro\n");
        printf("5. Devolver Livro\n");
        printf("6. Cadastrar Leitor\n");
        printf("7. Sair\n");
        printf("Escolha uma opção: ");
        scanf(" %d", &opcao);

        if(opcao == 1){
            cadastrarLivro(db);
        }else if(opcao == 2){
            buscarLivro(db);
        }else if(opcao == 3){
            listarLivros(db);
        }else if(opcao == 4){
            emprestarLivro(db);
        }else if(opcao == 5){
            devolverLivro(db);
        }else if(opcao == 6){
            cadastrarLeitor(db);
        }else if(opcao == 7){
            return 1;
        }
        else {
            printf("Opção invaĺida.Tente novamente!\n");
        }

    // Fechar o banco de dados
    sqlite3_close(db);

    return 0;
}
