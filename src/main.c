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
 * 功能：初始化程序运行环境。
 * 说明：程序源文件和提示文本均使用 UTF-8 保存。Windows 控制台默认代码页可能不是 UTF-8，
 *       需要同时设置 C 运行时本地化环境、控制台输入代码页和输出代码页，避免中文菜单乱码。
 */
static void init_runtime(void) {
#ifdef _WIN32
    if (setlocale(LC_ALL, ".UTF-8") == NULL) {
        setlocale(LC_ALL, "");
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    setlocale(LC_ALL, "");
#endif
}

/*
 * 功能：根据工资项目计算教师应发工资。
 * 参数：teacher 指向一条教师工资记录。
 * 返回：基本工资 + 岗位津贴 + 奖金 - 扣款。
 */
static double total_salary(const Teacher *teacher) {
    return teacher->baseSalary + teacher->allowance + teacher->bonus - teacher->deduction;
}

/*
 * 功能：去掉 fgets 读入字符串末尾的换行符。
 * 参数：text 为需要处理的字符串。
 */
static void trim_newline(char *text) {
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[--len] = '\0';
    }
}

/*
 * 功能：判断文本是否含有 CSV 分隔符逗号。
 * 参数：text 为用户输入的文本。
 * 返回：含逗号返回 1，否则返回 0。
 */
static int contains_comma(const char *text) {
    return strchr(text, ',') != NULL;
}

/*
 * 功能：安全读取一段非空文本。
 * 参数：prompt 为提示语；buffer 保存输入；size 为缓冲区长度。
 * 说明：禁止输入逗号，避免破坏 CSV 文件格式。
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
            puts("输入不能为空，请重新输入。");
            continue;
        }
        if (contains_comma(buffer)) {
            puts("输入内容不能包含英文逗号，请重新输入。");
            continue;
        }
        return;
    }
}

/*
 * 功能：读取指定范围内的整数。
 * 参数：prompt 为提示语；minValue/maxValue 为允许范围。
 * 返回：用户输入的合法整数。
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
        printf("请输入 %d 到 %d 之间的整数。\n", minValue, maxValue);
    }
}

/*
 * 功能：读取不小于指定最小值的浮点数。
 * 参数：prompt 为提示语；minValue 为最小允许值。
 * 返回：用户输入的合法金额。
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
        printf("请输入不小于 %.2f 的数字。\n", minValue);
    }
}

/*
 * 功能：创建教师链表节点。
 * 参数：teacher 为待保存的教师记录。
 * 返回：新创建的 TeacherNode 指针；内存不足时直接退出程序。
 */
static TeacherNode *create_teacher_node(Teacher teacher) {
    TeacherNode *node = (TeacherNode *)malloc(sizeof(TeacherNode));
    if (node == NULL) {
        perror("内存分配失败");
        exit(EXIT_FAILURE);
    }
    node->data = teacher;
    node->next = NULL;
    return node;
}

/*
 * 功能：创建查询结果链表节点。
 * 参数：teacher 为符合查询条件的教师记录副本。
 * 返回：新创建的 QueryNode 指针；内存不足时直接退出程序。
 */
static QueryNode *create_query_node(Teacher teacher) {
    QueryNode *node = (QueryNode *)malloc(sizeof(QueryNode));
    if (node == NULL) {
        perror("内存分配失败");
        exit(EXIT_FAILURE);
    }
    node->data = teacher;
    node->next = NULL;
    return node;
}

/*
 * 功能：把教师记录追加到教师链表尾部。
 * 参数：head 为链表头指针地址；teacher 为新增记录。
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
 * 功能：把一条记录追加到查询结果链表尾部。
 * 参数：head 为查询结果链表头指针地址；teacher 为查询命中的记录。
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
 * 功能：释放教师主链表占用的内存。
 * 参数：head 为教师链表头指针。
 */
static void free_teachers(TeacherNode *head) {
    while (head != NULL) {
        TeacherNode *next = head->next;
        free(head);
        head = next;
    }
}

/*
 * 功能：释放查询结果链表占用的内存。
 * 参数：head 为查询结果链表头指针。
 */
static void free_queries(QueryNode *head) {
    while (head != NULL) {
        QueryNode *next = head->next;
        free(head);
        head = next;
    }
}

/*
 * 功能：根据教师编号查找主链表中的记录。
 * 参数：head 为教师链表头指针；id 为教师编号。
 * 返回：找到则返回节点指针，否则返回 NULL。
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
 * 功能：打印工资信息表头。
 */
static void print_header(void) {
    puts("-----------------------------------------------------------------------------------------------");
    printf("%-10s %-10s %-6s %-6s %-12s %-12s %10s %10s %10s %10s %10s\n",
           "编号", "姓名", "性别", "年龄", "部门", "职称", "基本工资", "津贴", "奖金", "扣款", "应发工资");
    puts("-----------------------------------------------------------------------------------------------");
}

/*
 * 功能：按表格格式打印一条教师工资记录。
 * 参数：teacher 为需要输出的教师记录。
 */
static void print_teacher(const Teacher *teacher) {
    printf("%-10s %-10s %-6s %-6d %-12s %-12s %10.2f %10.2f %10.2f %10.2f %10.2f\n",
           teacher->id, teacher->name, teacher->gender, teacher->age, teacher->department, teacher->title,
           teacher->baseSalary, teacher->allowance, teacher->bonus, teacher->deduction, total_salary(teacher));
}

/*
 * 功能：输出查询结果链表中的所有记录。
 * 参数：head 为查询结果链表头指针。
 */
static void print_query_results(QueryNode *head) {
    int count = 0;
    if (head == NULL) {
        puts("未查询到符合条件的教师工资信息。");
        return;
    }

    print_header();
    for (QueryNode *current = head; current != NULL; current = current->next) {
        print_teacher(&current->data);
        count++;
    }
    puts("-----------------------------------------------------------------------------------------------");
    printf("共查询到 %d 条记录。\n", count);
}

/*
 * 功能：创建数据目录。
 * 返回：成功或目录已存在返回 1，失败返回 0。
 * 说明：同时兼容 Windows 的 _mkdir 和 Linux/macOS 的 mkdir，便于在不同系统编译运行。
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
    perror("创建 data 目录失败");
    return 0;
}

/*
 * 功能：把教师链表中的所有记录写入文件。
 * 参数：head 为教师链表头指针。
 * 返回：保存成功返回 1，失败返回 0。
 */
static int save_teachers(TeacherNode *head) {
    FILE *file;

    if (!ensure_data_directory()) {
        return 0;
    }

    file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        perror("保存文件失败");
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
 * 功能：解析文件中的一行 CSV 数据。
 * 参数：line 为一行文本；teacher 用于保存解析结果。
 * 返回：解析成功返回 1，字段数量不正确返回 0。
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
 * 功能：程序启动时从文件加载教师工资记录。
 * 参数：head 为教师链表头指针地址。
 */
static void load_teachers(TeacherNode **head) {
    FILE *file = fopen(DATA_FILE, "r");
    char line[LINE_SIZE];
    int loaded = 0;

    if (file == NULL) {
        puts("未找到历史数据文件，将从空数据开始。");
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
    printf("已从 %s 读取 %d 条教师工资记录。\n", DATA_FILE, loaded);
}

/*
 * 功能：录入一名教师的基本信息和工资信息。
 * 参数：head 为教师链表头指针地址。
 * 说明：录入后立即保存到文件，满足数据持久化要求。
 */
static void add_teacher(TeacherNode **head) {
    Teacher teacher;

    puts("\n【录入教师工资信息】");
    for (;;) {
        read_text("教师编号：", teacher.id, sizeof(teacher.id));
        if (find_teacher_by_id(*head, teacher.id) == NULL) {
            break;
        }
        puts("该教师编号已存在，请重新输入。");
    }

    read_text("姓名：", teacher.name, sizeof(teacher.name));
    read_text("性别：", teacher.gender, sizeof(teacher.gender));
    teacher.age = read_int("年龄：", 18, 100);
    read_text("所在部门：", teacher.department, sizeof(teacher.department));
    read_text("职称：", teacher.title, sizeof(teacher.title));
    teacher.baseSalary = read_double("基本工资：", 0.0);
    teacher.allowance = read_double("岗位津贴：", 0.0);
    teacher.bonus = read_double("奖金：", 0.0);
    teacher.deduction = read_double("扣款：", 0.0);

    append_teacher(head, teacher);
    if (save_teachers(*head)) {
        puts("录入成功，数据已保存到文件。");
    }
}

/*
 * 功能：按教师编号查询工资信息。
 * 参数：head 为教师链表头指针。
 * 说明：查询结果先保存到 QueryNode 链表，再统一输出。
 */
static void query_by_id(TeacherNode *head) {
    char id[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\n【按教师编号查询】");
    read_text("请输入教师编号：", id, sizeof(id));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.id, id) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * 功能：按所在部门查询工资信息。
 * 参数：head 为教师链表头指针。
 * 说明：同一部门可能有多名教师，结果使用查询链表保存并输出。
 */
static void query_by_department(TeacherNode *head) {
    char department[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\n【按所在部门查询】");
    read_text("请输入部门名称：", department, sizeof(department));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.department, department) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * 功能：按教师姓名查询工资信息。
 * 参数：head 为教师链表头指针。
 * 说明：这是扩展查询方式，支持同名教师，结果同样通过链表输出。
 */
static void query_by_name(TeacherNode *head) {
    char name[MAX_TEXT];
    QueryNode *results = NULL;

    puts("\n【按教师姓名查询】");
    read_text("请输入教师姓名：", name, sizeof(name));
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.name, name) == 0) {
            append_query(&results, current->data);
        }
    }

    print_query_results(results);
    free_queries(results);
}

/*
 * 功能：显示查询子菜单并调用具体查询函数。
 * 参数：head 为教师链表头指针。
 */
static void query_menu(TeacherNode *head) {
    int choice;
    do {
        puts("\n【教师工资查询】");
        puts("1. 按教师编号查询");
        puts("2. 按所在部门查询");
        puts("3. 按教师姓名查询");
        puts("0. 返回主菜单");
        choice = read_int("请选择：", 0, 3);
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
                puts("无效选择。");
        }
    } while (choice != 0);
}

/*
 * 功能：统计全校和各部门工资情况。
 * 参数：head 为教师链表头指针。
 * 输出：人数、工资总额、平均工资、最高工资、最低工资和部门汇总。
 */
static void statistics(TeacherNode *head) {
    int count = 0;
    double sum = 0.0;
    double maxSalary = 0.0;
    double minSalary = 0.0;
    const Teacher *maxTeacher = NULL;
    const Teacher *minTeacher = NULL;

    puts("\n【教师工资统计】");
    if (head == NULL) {
        puts("当前没有教师工资数据。\n");
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

    printf("教师总人数：%d\n", count);
    printf("工资总额：%.2f\n", sum);
    printf("平均工资：%.2f\n", sum / count);
    printf("最高工资：%.2f（%s %s）\n", maxSalary, maxTeacher->id, maxTeacher->name);
    printf("最低工资：%.2f（%s %s）\n", minSalary, minTeacher->id, minTeacher->name);

    puts("\n按部门统计：");
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
        printf("%-12s 人数：%d  工资总额：%.2f  平均工资：%.2f\n",
               outer->data.department, deptCount, deptSum, deptSum / deptCount);
    }
}

/*
 * 功能：按教师编号修改教师职称和工资项目。
 * 参数：head 为教师链表头指针。
 * 说明：适用于晋升职称、调整岗位津贴或奖金扣款等场景。
 */
static void modify_teacher(TeacherNode *head) {
    char id[MAX_TEXT];
    TeacherNode *node;

    puts("\n【教师工资修改】");
    read_text("请输入需要修改的教师编号：", id, sizeof(id));
    node = find_teacher_by_id(head, id);
    if (node == NULL) {
        puts("未找到该教师编号。");
        return;
    }

    puts("当前信息：");
    print_header();
    print_teacher(&node->data);
    puts("-----------------------------------------------------------------------------------------------");

    read_text("新职称：", node->data.title, sizeof(node->data.title));
    node->data.baseSalary = read_double("新基本工资：", 0.0);
    node->data.allowance = read_double("新岗位津贴：", 0.0);
    node->data.bonus = read_double("新奖金：", 0.0);
    node->data.deduction = read_double("新扣款：", 0.0);

    if (save_teachers(head)) {
        puts("修改成功，数据已保存到文件。");
    }
}

/*
 * 功能：按教师编号删除一条记录。
 * 参数：head 为教师链表头指针地址。
 * 说明：这是扩展维护功能，删除后立即写回文件。
 */
static void delete_teacher(TeacherNode **head) {
    char id[MAX_TEXT];
    TeacherNode *current = *head;
    TeacherNode *previous = NULL;

    puts("\n【删除教师工资信息】");
    read_text("请输入需要删除的教师编号：", id, sizeof(id));

    while (current != NULL && strcmp(current->data.id, id) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        puts("未找到该教师编号。");
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }
    free(current);

    if (save_teachers(*head)) {
        puts("删除成功，数据已保存到文件。");
    }
}

/*
 * 功能：显示所有教师工资信息。
 * 参数：head 为教师链表头指针。
 * 说明：复用查询结果链表输出，保持与查询功能一致的输出形式。
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
 * 功能：打印主菜单。
 */
static void print_menu(void) {
    puts("\n========== 教师工资管理系统 ==========");
    puts("1. 录入教师工资信息");
    puts("2. 查询教师工资信息");
    puts("3. 统计教师工资信息");
    puts("4. 修改教师工资信息");
    puts("5. 显示全部教师工资信息");
    puts("6. 删除教师工资信息");
    puts("0. 退出系统");
}

/*
 * 功能：程序入口函数。
 * 流程：初始化运行环境、加载文件数据、循环显示菜单、按用户选择调用功能、退出前释放链表内存。
 */
int main(void) {
    TeacherNode *teachers = NULL;
    int choice;

    init_runtime();
    load_teachers(&teachers);
    do {
        print_menu();
        choice = read_int("请选择功能：", 0, 6);
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
                puts("感谢使用，再见！");
                break;
            default:
                puts("无效选择，请重新输入。");
        }
    } while (choice != 0);

    free_teachers(teachers);
    return 0;
}
