#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reverse_string(char *str)
{
    int len = strlen(str);

    for(int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main()
{
    char username[100];
    char password[100];

    printf("Username: ");
    scanf("%99s", username);

    printf("Password: ");
    scanf("%99s", password);

    FILE *fp = fopen("ftp_cmd.txt", "w");

    fprintf(fp, "user %s %s\n", username, password);
    fprintf(fp, "ls\n");
    fprintf(fp, "quit\n");

    fclose(fp);

    system("ftp -inv lebavui.io.vn < ftp_cmd.txt > ftp_output.txt");

    FILE *out = fopen("ftp_output.txt", "r");

    char line[1024];
    char question[256] = "";

    while(fgets(line, sizeof(line), out))
    {
        if(strstr(line, "question_") && strstr(line, ".txt"))
        {
            sscanf(line, "%*s %*s %*s %*s %*s %*s %*s %*s %255s", question);
            question[strcspn(question, "\r\n")] = '\0';
            break;
        }
    }

    fclose(out);

    if(strlen(question) == 0)
    {
        printf("Question file not found\n");
        return 1;
    }

    printf("Found: %s\n", question);

    fp = fopen("ftp_cmd.txt", "w");

    fprintf(fp, "user %s %s\n", username, password);
    fprintf(fp, "get %s\n", question);
    fprintf(fp, "quit\n");

    fclose(fp);

    system("ftp -inv lebavui.io.vn < ftp_cmd.txt");

    FILE *qf = fopen(question, "r");

    if(qf == NULL)
    {
        printf("Cannot download question file\n");
        return 1;
    }

    char content[2048];

    fgets(content, sizeof(content), qf);

    content[strcspn(content, "\r\n")] = '\0';

    fclose(qf);

    reverse_string(content);

    char answer[256];

    snprintf(answer, sizeof(answer), "answer_%s", question + 9);

    FILE *af = fopen(answer, "w");

    fprintf(af, "%s", content);

    fclose(af);

    printf("Created: %s\n", answer);

    fp = fopen("ftp_cmd.txt", "w");

    fprintf(fp, "user %s %s\n", username, password);
    fprintf(fp, "put %s\n", answer);
    fprintf(fp, "quit\n");

    fclose(fp);

    system("ftp -inv lebavui.io.vn < ftp_cmd.txt");

    printf("Upload complete\n");

    remove("ftp_cmd.txt");

    return 0;
}