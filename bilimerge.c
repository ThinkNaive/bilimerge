#include "bilimerge.h"

#define TYPE_LNK 0x00
#define TYPE_NUM 0x01
#define TYPE_BOL 0x02
#define TYPE_STR 0x03
typedef int type;
typedef __uint8_t Dtype;

typedef struct TJSON
{
    char name[1024];
    type tag;
    union {
        bool value;
        int data;
        char string[1024];
    };
    struct TJSON *child;
    struct TJSON *sibling;
} Tjson;

typedef struct FLVLIST
{
    int key;
    char name[1024];
} Flvlist;

bool isConnectCh(int c)
{
    return c == ' ' || c == '\r' || c == '\n';
}

bool StrCmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1++ == *s2++)
        ;
    return !(*s1 || *s2);
}

Tjson *ReadJson(FILE *f)
{
    if (feof(f))
        return NULL;
    int c = fgetc(f);

    while (isConnectCh(c) && !feof(f))
        c = fgetc(f);
    if (isConnectCh(c))
        return NULL;
    Tjson *t = (Tjson *)malloc(sizeof(Tjson));

    // create name
    int top = -1;
    c = fgetc(f);
    while (c != '\"' && !feof(f))
    {
        if (c == '\\')
        {
            c = fgetc(f);
        }
        t->name[++top] = c;
        c = fgetc(f);
    }
    t->name[++top] = '\0';

    // go to find :
    while (isConnectCh(c) && !feof(f))
        c = fgetc(f);
    c = fgetc(f);
    c = ' ';
    while (isConnectCh(c) && !feof(f))
        c = fgetc(f);

    // analyse element value
    if (c == 't') // true
    {
        fseek(f, 3, 1);
        t->tag = TYPE_BOL;
        t->value = true;
        t->child = NULL;
    }
    else if (c == 'f') // false
    {
        fseek(f, 4, 1);
        t->tag = TYPE_BOL;
        t->value = false;
        t->child = NULL;
    }
    else if (c >= '0' && c <= '9') // number
    {
        fseek(f, -1, 1);
        t->tag = TYPE_NUM;
        fscanf(f, "%d", &(t->data));
        t->child = NULL;
    }
    else if (c == '\"') // string
    {
        top = -1;
        c = fgetc(f);
        while (c != '\"' && !feof(f))
        {
            if (c == '\\')
            {
                c = fgetc(f);
            }
            t->string[++top] = c;
            c = fgetc(f);
        }
        t->string[++top] = '\0';
        t->tag = TYPE_STR;
        t->child = NULL;
    }
    else if (c == '{')
    {
        t->tag = TYPE_LNK;
        t->child = ReadJson(f);
    }

    c = ' ';
    c = fgetc(f);
    while (isConnectCh(c) && !feof(f))
        c = fgetc(f);

    if (c == '}')
        t->sibling = NULL;
    else if (c == ',')
        t->sibling = ReadJson(f);
    return t;
}

Tjson *CreateJson(FILE *f) // call function
{
    fgetc(f);
    Tjson *t = (Tjson *)malloc(sizeof(Tjson));
    t->sibling = NULL;
    strcpy(t->name, "root");
    t->tag = TYPE_LNK;
    t->child = ReadJson(f);
    return t;
}

Tjson *Find(Tjson *t, char key[], bool *finish)
{
    if (!t)
        return NULL;
    Tjson *p = t;
    if (StrCmp(key, t->name))
        *finish = true;
    if (!(*finish))
        p = Find(t->child, key, finish);
    if (!(*finish))
        p = Find(t->sibling, key, finish);
    return p;
}

Tjson *GetJsonNode(Tjson *t, char key[])
{
    bool finish = false;
    return Find(t, key, &finish);
}

void DeleteJson(Tjson *t)
{
    if (!t)
        return;
    DeleteJson(t->child);
    DeleteJson(t->sibling);
    free(t);
    t = NULL;
}

bool GetDirList(char *basePath, Dtype dtype, char stack[][1024], int *top)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1024];

    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return false;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        else if (ptr->d_type == dtype)
        {
            strcpy(stack[++(*top)], basePath);
            strcat(stack[*top], "/");
            strcat(stack[*top], ptr->d_name);
        }
    }
    closedir(dir);
    return true;
}

bool GetFileSpec(char *basePath, Dtype dtype, char suffix[1024], char stack[][1024], int *top)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1024];

    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return false;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        else if (ptr->d_type == dtype)
        {
            int finish = strlen(suffix) - 1;
            int pos = strlen(ptr->d_name) - 1;
            while (finish > -1 && pos > -1 && suffix[finish--] == ptr->d_name[pos--])
                ;
            if (finish > -1)
                continue; // 后缀不匹配
            strcpy(stack[++(*top)], ptr->d_name);
        }
    }
    closedir(dir);
    return true;
}

void sort(Flvlist list[], int n)
{
    bool finish = false;
    while (!finish)
    {
        finish = true;
        for (int i = 0; i < n - 1; ++i)
            if (list[i].key > list[i + 1].key)
            {
                finish = false;
                Flvlist temp;
                temp.key = list[i].key;
                strcpy(temp.name, list[i].name);
                list[i].key = list[i + 1].key;
                strcpy(list[i].name, list[i + 1].name);
                list[i + 1].key = temp.key;
                strcpy(list[i + 1].name, temp.name);
            }
        n--;
    }
}

int main()
{
    char stack[1024][1024];
    int top = -1;
    char stpara[4096][1024];
    int toppara = -1;
    GetDirList(".", DT_DIR, stack, &top);
    while (top > -1)
        GetDirList(stack[top--], DT_DIR, stpara, &toppara);
    int count = toppara + 1;

    while (toppara > -1)
    {
        char str_ent[1024];
        char fileHead[1024];
        memset(fileHead, 0, 1024 * sizeof(char));
        fileHead[0] = '.';
        fileHead[1] = '/';
        strcpy(str_ent, stpara[toppara]);
        strcat(str_ent, "/entry.json");
        FILE *f = fopen(str_ent, "r");
        Tjson *t = CreateJson(f);
        Tjson *e;
        e = GetJsonNode(t, "title");
        strcat(fileHead, e->string);
        e = GetJsonNode(t, "part");
        strcat(fileHead, "- ");
        strcat(fileHead, e->string);
        strcat(fileHead, ".flv");
        e = GetJsonNode(t, "type_tag"); // child directory
        strcat(stpara[toppara], "/");
        strcat(stpara[toppara], e->string);
        DeleteJson(t);

        GetFileSpec(stpara[toppara], DT_REG, ".blv", stack, &top);
        // 对文件排序
        Flvlist flvlist[top + 1];
        int listcount = top + 1;
        while (top > -1)
        {
            sscanf(stack[top], "%d.blv", &flvlist[top].key);
            strcpy(flvlist[top].name, stack[top]);
            top--;
        }

        sort(flvlist, listcount);
        char *argv[listcount + 2];
        argv[0] = (char *)malloc(1024 * sizeof(char));
        argv[1] = (char *)malloc(1024 * sizeof(char));
        strcpy(argv[1], fileHead);
        for (int i = 0; i < listcount; ++i)
        {
            argv[i + 2] = (char *)malloc(1024 * sizeof(char));
            strcpy(argv[i + 2], stpara[toppara]);
            strcat(argv[i + 2], "/");
            strcat(argv[i + 2], flvlist[i].name);
        }

        flvmerge(listcount + 2, argv);
        printf("Completed : %5.1f%%\r", (float)100.0*(count - toppara)/count);

        for (int i = 0; i < listcount + 2; ++i)
            free(argv[i]);
        toppara--;
    }

    return 0;
}