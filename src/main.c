#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static double total_salary(const Teacher *teacher) {
    return teacher->baseSalary + teacher->allowance + teacher->bonus - teacher->deduction;
}

static void trim_newline(char *text) {
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[--len] = '\0';
    }
}

static void read_text(const char *prompt, char *buffer, size_t size) {
    for (;;) {
        printf("%s", prompt);
        if (fgets(buffer, (int)size, stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        trim_newline(buffer);
        if (buffer[0] != '\0') {
            return;
        }
        puts("输入不能为空，请重新输入。");
    }
}

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

static void free_teachers(TeacherNode *head) {
    while (head != NULL) {
        TeacherNode *next = head->next;
        free(head);
        head = next;
    }
}

static void free_queries(QueryNode *head) {
    while (head != NULL) {
        QueryNode *next = head->next;
        free(head);
        head = next;
    }
}

static TeacherNode *find_teacher_by_id(TeacherNode *head, const char *id) {
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->data.id, id) == 0) {
            return current;
        }
    }
    return NULL;
}

static void print_header(void) {
    puts("-----------------------------------------------------------------------------------------------");
    printf("%-10s %-10s %-6s %-6s %-12s %-12s %10s %10s %10s %10s %10s\n",
           "编号", "姓名", "性别", "年龄", "部门", "职称", "基本工资", "津贴", "奖金", "扣款", "应发工资");
    puts("-----------------------------------------------------------------------------------------------");
}

static void print_teacher(const Teacher *teacher) {
    printf("%-10s %-10s %-6s %-6d %-12s %-12s %10.2f %10.2f %10.2f %10.2f %10.2f\n",
           teacher->id, teacher->name, teacher->gender, teacher->age, teacher->department, teacher->title,
           teacher->baseSalary, teacher->allowance, teacher->bonus, teacher->deduction, total_salary(teacher));
}

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

static int ensure_data_directory(void) {
    FILE *probe = fopen("data/.keep", "a");
    if (probe == NULL) {
        puts("无法访问 data 目录，请确认程序在项目根目录下运行。");
        return 0;
    }
    fclose(probe);
    return 1;
}

static int save_teachers(TeacherNode *head) {
    if (!ensure_data_directory()) {
        return 0;
    }

    FILE *file = fopen(DATA_FILE, "w");
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

static void query_menu(TeacherNode *head) {
    int choice;
    do {
        puts("\n【教师工资查询】");
        puts("1. 按教师编号查询");
        puts("2. 按所在部门查询");
        puts("0. 返回主菜单");
        choice = read_int("请选择：", 0, 2);
        switch (choice) {
            case 1:
                query_by_id(head);
                break;
            case 2:
                query_by_department(head);
                break;
            case 0:
                break;
            default:
                puts("无效选择。");
        }
    } while (choice != 0);
}

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

static void list_all(TeacherNode *head) {
    QueryNode *results = NULL;
    for (TeacherNode *current = head; current != NULL; current = current->next) {
        append_query(&results, current->data);
    }
    print_query_results(results);
    free_queries(results);
}

static void print_menu(void) {
    puts("\n========== 教师工资管理系统 ==========");
    puts("1. 录入教师工资信息");
    puts("2. 查询教师工资信息");
    puts("3. 统计教师工资信息");
    puts("4. 修改教师工资信息");
    puts("5. 显示全部教师工资信息");
    puts("0. 退出系统");
}

int main(void) {
    TeacherNode *teachers = NULL;
    int choice;

    load_teachers(&teachers);
    do {
        print_menu();
        choice = read_int("请选择功能：", 0, 5);
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
