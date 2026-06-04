#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define DATA_DIR "data"
#define DATA_FILE "data/teachers.csv"
#define MAX_TEXT 64
#define LINE_SIZE 512

typedef struct {
    char id[MAX_TEXT];
    char name[MAX_TEXT];
    char gender[MAX_TEXT];
    int age;
    char department[MAX_TEXT];
    char title[MAX_TEXT];
    double baseSalary;
    double allowance;
    double bonus;
    double deduction;
} Teacher;

typedef struct TeacherNode {
    Teacher data;
    struct TeacherNode *next;
} TeacherNode;

typedef struct QueryNode {
    Teacher data;
    struct QueryNode *next;
} QueryNode;

/*
 * ܣʼл
 * ˵Դļʾıʹ GBK 档Windows Ŀ̨ôҳΪ 936
 *       ͬ C ʱػ̨ҳҳĲ˵롣
 */
static void init_runtime(void) {
#ifdef _WIN32
    if (setlocale(LC_ALL, ".936") == NULL) {
        setlocale(LC_ALL, "");
    }
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
#else
    setlocale(LC_ALL, "");
#endif
}

/*
 * ܣݹĿʦӦʡ
 * teacher ָһʦʼ¼
 * أ + λ +  - ۿ
 */
static double total_salary(const Teacher *teacher) {
    return teacher->baseSalary + teacher->allowance + teacher->bonus - teacher->deduction;
}

/*
 * ܣȥ fgets ַĩβĻз
 * text ΪҪַ
 */
static void trim_newline(char *text) {
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[--len] = '\0';
    }
}

/*
 * ܣжıǷ CSV ָš
 * text Ϊûı
 * أŷ 1򷵻 0
 */
static int contains_comma(const char *text) {
    return strchr(text, ',') != NULL;
}

/*
 * ܣȫȡһηǿı
 * prompt Ϊʾbuffer 룻size Ϊȡ
 * ˵ֹ붺ţƻ CSV ļʽ
 */
static void read_text(const char *prompt, char *buffer, size_t size) {
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(buffer, (int)size, stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        trim_newline(buffer);
        if (buffer[0] == '\0') {
            puts("벻Ϊգ롣");
            continue;
        }
        if (contains_comma(buffer)) {
            puts("ݲܰӢĶţ롣");
            continue;
        }
        return;
    }
}

/*
 * ܣȡָΧڵ
 * prompt ΪʾminValue/maxValue ΪΧ
 * أûĺϷ
 */
static int read_int(const char *prompt, int minValue, int maxValue) {
    char buffer[MAX_TEXT];
    char *end = NULL;
    long value;

    for (;;) {
        read_text(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtol(buffer, &end, 10);
        if (errno == 0 && end != buffer && *end == '\0' && value >= minValue && value <= maxValue) {
            return (int)value;
        }
        printf(" %d  %d ֮\n", minValue, maxValue);
    }
}

/*
 * ܣȡСָСֵĸ
 * prompt ΪʾminValue ΪСֵ
 * أûĺϷ
 */
static double read_double(const char *prompt, double minValue) {
    char buffer[MAX_TEXT];
    char *end = NULL;
    double value;

    for (;;) {
        read_text(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtod(buffer, &end);
        if (errno == 0 && end != buffer && *end == '\0' && value >= minValue) {
            return value;
        }
        printf("벻С %.2f ֡\n", minValue);
    }
}

/*
 * ܣʦڵ㡣
 * teacher ΪĽʦ¼
 * أ´ TeacherNode ָ룻ڴ治ʱֱ˳
 */
static TeacherNode *create_teacher_node(Teacher teacher) {
    TeacherNode *node = (TeacherNode *)malloc(sizeof(TeacherNode));
    if (node == NULL) {
        perror("ڴʧ");
        exit(EXIT_FAILURE);
    }
    node->data = teacher;
    node->next = NULL;
    return node;
}

/*
 * ܣѯڵ㡣
 * teacher ΪϲѯĽʦ¼
 * أ´ QueryNode ָ룻ڴ治ʱֱ˳
 */
static QueryNode *create_query_node(Teacher teacher) {
    QueryNode *node = (QueryNode *)malloc(sizeof(QueryNode));
    if (node == NULL) {
        perror("ڴʧ");
        exit(EXIT_FAILURE);
    }
    node->data = teacher;
    node->next = NULL;
    return node;
}

/*
 * ܣѽʦ¼׷ӵʦβ
 * head Ϊͷַָteacher Ϊ¼
 */
static void append_teacher(TeacherNode **head, Teacher teacher) {
    TeacherNode *node = create_teacher_node(teacher);
    if (*head == NULL) {
        *head = node;
        return;
    }

    TeacherNode *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

/*
 * ܣһ¼׷ӵѯβ
 * head Ϊѯͷַָteacher Ϊѯеļ¼
 */
static void append_query(QueryNode **head, Teacher teacher) {
    QueryNode *node = create_query_node(teacher);
    if (*head == NULL) {
        *head = node;
        return;
    }

    QueryNode *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

/*
 * ܣͷŽʦռõڴ档
 * head Ϊʦͷָ롣
 */
static void free_teachers(TeacherNode *head) {
    while (head != NULL) {
        TeacherNode *next = head->next;
        free(head);
        head = next;
    }
}

/*
 * ܣͷŲѯռõڴ档
 * head Ϊѯͷָ롣
 */
static void free_queries(QueryNode *head) {
    while (head != NULL) {
        QueryNode *next = head->next;
        free(head);
        head = next;
    }
}

/*
 * ܣݽʦŲеļ¼
 * head Ϊʦͷָ룻id Ϊʦš
 * أҵ򷵻ؽڵָ룬򷵻 NULL
 */
static TeacherNode *find_teacher_by_id(TeacherNode *head, const char *id) {
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.id, id) == 0) {
            return current;
        }
    }
    return NULL;
}

/*
 * ܣӡϢͷ
 */
static void print_header(void) {
    puts("-----------------------------------------------------------------------------------------------");
    printf("%-10s %-10s %-6s %-6s %-12s %-12s %10s %10s %10s %10s %10s\n",
           "", "", "Ա", "", "", "ְ", "", "", "", "ۿ", "Ӧ");
    puts("-----------------------------------------------------------------------------------------------");
}

/*
 * ܣʽӡһʦʼ¼
 * teacher ΪҪĽʦ¼
 */
static void print_teacher(const Teacher *teacher) {
    printf("%-10s %-10s %-6s %-6d %-12s %-12s %10.2f %10.2f %10.2f %10.2f %10.2f\n",
           teacher->id, teacher->name, teacher->gender, teacher->age, teacher->department, teacher->title,
           teacher->baseSalary, teacher->allowance, teacher->bonus, teacher->deduction, total_salary(teacher));
}

/*
 * ܣѯем¼
 * head Ϊѯͷָ롣
 */
static void print_query_results(QueryNode *head) {
    int count = 0;
    if (head == NULL) {
        puts("δѯĽʦϢ");
        return;
    }

    print_header();
    for (QueryNode *current = head; current != NULL; current = current->next) {
        print_teacher(&current->data);
        count++;
    }
    puts("-----------------------------------------------------------------------------------------------");
    printf("ѯ %d ¼\n", count);
}

/*
 * ܣĿ¼
 * أɹĿ¼Ѵڷ 1ʧܷ 0
 * ˵ͬʱ Windows  _mkdir  Linux/macOS  mkdirڲͬϵͳС
 */
static int ensure_data_directory(void) {
#ifdef _WIN32
    if (_mkdir(DATA_DIR) == 0 || errno == EEXIST) {
        return 1;
    }
#else
    if (mkdir(DATA_DIR, 0755) == 0 || errno == EEXIST) {
        return 1;
    }
#endif
    perror(" data Ŀ¼ʧ");
    return 0;
}

/*
 * ܣѽʦем¼дļ
 * head Ϊʦͷָ롣
 * أɹ 1ʧܷ 0
 */
static int save_teachers(TeacherNode *head) {
    FILE *file;

    if (!ensure_data_directory()) {
        return 0;
    }

    file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        perror("ļʧ");
        return 0;
    }

    for (TeacherNode *current = head; current != NULL; current = current->next) {
        Teacher *t = &current->data;
        fprintf(file, "%s,%s,%s,%d,%s,%s,%.2f,%.2f,%.2f,%.2f\n",
                t->id, t->name, t->gender, t->age, t->department, t->title,
                t->baseSalary, t->allowance, t->bonus, t->deduction);
    }

    fclose(file);
    return 1;
}

/*
 * ܣļеһ CSV ݡ
 * line Ϊһıteacher ڱ
 * أɹ 1ֶȷ 0
 */
static int parse_teacher_line(char *line, Teacher *teacher) {
    char *fields[10];
    int index = 0;
    char *token = strtok(line, ",");

    while (token != NULL && index < 10) {
        fields[index++] = token;
        token = strtok(NULL, ",");
    }
    if (index != 10) {
        return 0;
    }

    snprintf(teacher->id, sizeof(teacher->id), "%s", fields[0]);
    snprintf(teacher->name, sizeof(teacher->name), "%s", fields[1]);
    snprintf(teacher->gender, sizeof(teacher->gender), "%s", fields[2]);
    teacher->age = atoi(fields[3]);
    snprintf(teacher->department, sizeof(teacher->department), "%s", fields[4]);
    snprintf(teacher->title, sizeof(teacher->title), "%s", fields[5]);
    teacher->baseSalary = atof(fields[6]);
    teacher->allowance = atof(fields[7]);
    teacher->bonus = atof(fields[8]);
    teacher->deduction = atof(fields[9]);
    return 1;
}

/*
 * ܣʱļؽʦʼ¼
 * head Ϊʦͷַָ
 */
static void load_teachers(TeacherNode **head) {
    FILE *file = fopen(DATA_FILE, "r");
    char line[LINE_SIZE];
    int loaded = 0;

    if (file == NULL) {
        puts("δҵʷļӿݿʼ");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        Teacher teacher;
        trim_newline(line);
        if (line[0] == '\0') {
            continue;
        }
        if (parse_teacher_line(line, &teacher)) {
            append_teacher(head, teacher);
            loaded++;
        }
    }

    fclose(file);
    printf("Ѵ %s ȡ %d ʦʼ¼\n", DATA_FILE, loaded);
}

/*
 * ܣ¼һʦĻϢ͹Ϣ
 * head Ϊʦͷַָ
 * ˵¼浽ļݳ־ûҪ
 */
static void add_teacher(TeacherNode **head) {
    Teacher teacher;

    puts("\n¼ʦϢ");
    for (;;) {
        read_text("ʦţ", teacher.id, sizeof(teacher.id));
        if (find_teacher_by_id(*head, teacher.id) == NULL) {
            break;
        }
        puts("ýʦѴڣ롣");
    }

    read_text("", teacher.name, sizeof(teacher.name));
    read_text("Ա", teacher.gender, sizeof(teacher.gender));
    teacher.age = read_int("䣺", 18, 100);
    read_text("ڲţ", teacher.department, sizeof(teacher.department));
    read_text("ְƣ", teacher.title, sizeof(teacher.title));
    teacher.baseSalary = read_double("ʣ", 0.0);
    teacher.allowance = read_double("λ", 0.0);
    teacher.bonus = read_double("", 0.0);
    teacher.deduction = read_double("ۿ", 0.0);

    append_teacher(head, teacher);
    if (save_teachers(*head)) {
        puts("¼ɹѱ浽ļ");
    }
}

/*
 * ܣʦŲѯϢ
 * head Ϊʦͷָ롣
 * ˵ѯȱ浽 QueryNode ͳһ
 */
static void query_by_id(TeacherNode *head) {
    char id[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\nʦŲѯ");
    read_text("ʦţ", id, sizeof(id));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.id, id) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * ܣڲŲѯϢ
 * head Ϊʦͷָ롣
 * ˵ͬһſжʦʹòѯ沢
 */
static void query_by_department(TeacherNode *head) {
    char department[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\nڲŲѯ");
    read_text("벿ƣ", department, sizeof(department));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.department, department) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * ܣʦѯϢ
 * head Ϊʦͷָ롣
 * ˵չѯʽ֧ͬʦͬͨ
 */
static void query_by_name(TeacherNode *head) {
    char name[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\nʦѯ");
    read_text("ʦ", name, sizeof(name));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.name, name) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * ܣʾѯӲ˵þѯ
 * head Ϊʦͷָ롣
 */
static void query_menu(TeacherNode *head) {
    int choice;
    do {
        puts("\nʦʲѯ");
        puts("1. ʦŲѯ");
        puts("2. ڲŲѯ");
        puts("3. ʦѯ");
        puts("0. ˵");
        choice = read_int("ѡ", 0, 3);
        switch (choice) {
            case 1:
                query_by_id(head);
                break;
            case 2:
                query_by_department(head);
                break;
            case 3:
                query_by_name(head);
                break;
            case 0:
                break;
            default:
                puts("Чѡ");
        }
    } while (choice != 0);
}

/*
 * ܣͳȫУ͸Ź
 * head Ϊʦͷָ롣
 * ܶƽʡ߹ʡ͹ʺͲŻܡ
 */
static void statistics(TeacherNode *head) {
    int count = 0;
    double sum = 0.0;
    double maxSalary = 0.0;
    double minSalary = 0.0;
    const Teacher *maxTeacher = NULL;
    const Teacher *minTeacher = NULL;

    puts("\nʦͳơ");
    if (head == NULL) {
        puts("ǰûнʦݡ\n");
        return;
    }

    for (TeacherNode *current = head; current != NULL; current = current->next) {
        double salary = total_salary(&current->data);
        if (count == 0 || salary > maxSalary) {
            maxSalary = salary;
            maxTeacher = &current->data;
        }
        if (count == 0 || salary < minSalary) {
            minSalary = salary;
            minTeacher = &current->data;
        }
        sum += salary;
        count++;
    }

    printf("ʦ%d\n", count);
    printf("ܶ%.2f\n", sum);
    printf("ƽʣ%.2f\n", sum / count);
    printf("߹ʣ%.2f%s %s\n", maxSalary, maxTeacher->id, maxTeacher->name);
    printf("͹ʣ%.2f%s %s\n", minSalary, minTeacher->id, minTeacher->name);

    puts("\nͳƣ");
    for (TeacherNode *outer = head; outer != NULL; outer = outer->next) {
        int seen = 0;
        int deptCount = 0;
        double deptSum = 0.0;

        for (TeacherNode *before = head; before != outer; before = before->next) {
            if (strcmp(before->data.department, outer->data.department) == 0) {
                seen = 1;
                break;
            }
        }
        if (seen) {
            continue;
        }

        for (TeacherNode *inner = head; inner != NULL; inner = inner->next) {
            if (strcmp(inner->data.department, outer->data.department) == 0) {
                deptCount++;
                deptSum += total_salary(&inner->data);
            }
        }
        printf("%-12s %d  ܶ%.2f  ƽʣ%.2f\n",
               outer->data.department, deptCount, deptSum, deptSum / deptCount);
    }
}

/*
 * ܣʦ޸Ľʦְƺ͹Ŀ
 * head Ϊʦͷָ롣
 * ˵ڽְơλ򽱽ۿȳ
 */
static void modify_teacher(TeacherNode *head) {
    char id[MAX_TEXT];
    TeacherNode *node;

    puts("\nʦ޸ġ");
    read_text("Ҫ޸ĵĽʦţ", id, sizeof(id));
    node = find_teacher_by_id(head, id);
    if (node == NULL) {
        puts("δҵýʦš");
        return;
    }

    puts("ǰϢ");
    print_header();
    print_teacher(&node->data);
    puts("-----------------------------------------------------------------------------------------------");

    read_text("ְƣ", node->data.title, sizeof(node->data.title));
    node->data.baseSalary = read_double("»ʣ", 0.0);
    node->data.allowance = read_double("¸λ", 0.0);
    node->data.bonus = read_double("½", 0.0);
    node->data.deduction = read_double("¿ۿ", 0.0);

    if (save_teachers(head)) {
        puts("޸ĳɹѱ浽ļ");
    }
}

/*
 * ܣʦɾһ¼
 * head Ϊʦͷַָ
 * ˵չάܣɾдļ
 */
static void delete_teacher(TeacherNode **head) {
    char id[MAX_TEXT];
    TeacherNode *current = *head;
    TeacherNode *previous = NULL;

    puts("\nɾʦϢ");
    read_text("ҪɾĽʦţ", id, sizeof(id));

    while (current != NULL && strcmp(current->data.id, id) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        puts("δҵýʦš");
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }
    free(current);

    if (save_teachers(*head)) {
        puts("ɾɹѱ浽ļ");
    }
}

/*
 * ܣʾнʦϢ
 * head Ϊʦͷָ롣
 * ˵òѯѯһµʽ
 */
static void list_all(TeacherNode *head) {
    QueryNode *results = NULL;
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        append_query(&results, current->data);
    }
    print_query_results(results);
    free_queries(results);
}

/*
 * ܣӡ˵
 */
static void print_menu(void) {
    puts("\n========== ʦʹϵͳ ==========");
    puts("1. ¼ʦϢ");
    puts("2. ѯʦϢ");
    puts("3. ͳƽʦϢ");
    puts("4. ޸ĽʦϢ");
    puts("5. ʾȫʦϢ");
    puts("6. ɾʦϢ");
    puts("0. ˳ϵͳ");
}

/*
 * ܣں
 * ̣ʼлļݡѭʾ˵ûѡùܡ˳ǰͷڴ档
 */
int main(void) {
    TeacherNode *teachers = NULL;
    int choice;

    init_runtime();
    load_teachers(&teachers);
    do {
        print_menu();
        choice = read_int("ѡܣ", 0, 6);
        switch (choice) {
            case 1:
                add_teacher(&teachers);
                break;
            case 2:
                query_menu(teachers);
                break;
            case 3:
                statistics(teachers);
                break;
            case 4:
                modify_teacher(teachers);
                break;
            case 5:
                list_all(teachers);
                break;
            case 6:
                delete_teacher(&teachers);
                break;
            case 0:
                puts("лʹãټ");
                break;
            default:
                puts("Чѡ롣");
        }
    } while (choice != 0);

    free_teachers(teachers);
    return 0;
}
