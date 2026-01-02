#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define MAX_SUBJECTS 8
#define MAX_ACCOUNTS 120
#define NAME_LEN 32
#define DAYS 7
#define HOURS 24
#define DATA_FILE "data.bin"
#to begin the session you should login with user name admin and psw admin

typedef enum { ROLE_ADMIN=0, ROLE_STUDENT=1 } Role;

typedef struct {
    int id;
    char name[NAME_LEN];
    int scores[MAX_SUBJECTS];
    int freeTime[DAYS][HOURS];
    double reputation;
    int ratingCount;
} Student;

typedef struct {
    char username[NAME_LEN];
    char password[NAME_LEN];
    Role role;
    int studentId;
} Account;

/* GLOBAL DATA */
Student students[MAX_STUDENTS];
Account accounts[MAX_ACCOUNTS];
char subjects[MAX_SUBJECTS][NAME_LEN];

int studentCount=0, accountCount=0, subjectCount=0, nextStudentId=1;

/* UTILS */
void readLine(char *b,int n){if(fgets(b,n,stdin)){size_t l=strlen(b);if(l&&b[l-1]=='\n')b[l-1]=0;}}
Student* findStudent(int id){for(int i=0;i<studentCount;i++)if(students[i].id==id)return &students[i];return NULL;}

/* SAVE DATA */
void saveData(){
    FILE *f=fopen(DATA_FILE,"wb"); if(!f) return;
    fwrite(&studentCount,sizeof(int),1,f);
    fwrite(students,sizeof(Student),studentCount,f);
    fwrite(&accountCount,sizeof(int),1,f);
    fwrite(accounts,sizeof(Account),accountCount,f);
    fwrite(&subjectCount,sizeof(int),1,f);
    fwrite(subjects,sizeof(subjects),1,f);
    fwrite(&nextStudentId,sizeof(int),1,f);
    fclose(f);
    printf("Data saved\n");
}

/* LOAD DATA */
void loadData(){
    FILE *f=fopen(DATA_FILE,"rb"); if(!f) return;
    fread(&studentCount,sizeof(int),1,f);
    fread(students,sizeof(Student),studentCount,f);
    fread(&accountCount,sizeof(int),1,f);
    fread(accounts,sizeof(Account),accountCount,f);
    fread(&subjectCount,sizeof(int),1,f);
    fread(subjects,sizeof(subjects),subjectCount,f);
    fread(&nextStudentId,sizeof(int),1,f);
    fclose(f);
}

/* LOGIN */
Account* login(){
    char u[NAME_LEN],p[NAME_LEN];
    printf("Username: "); readLine(u,NAME_LEN);
    printf("Password: "); readLine(p,NAME_LEN);
    for(int i=0;i<accountCount;i++)
        if(!strcmp(accounts[i].username,u) && !strcmp(accounts[i].password,p))
            return &accounts[i];
    return NULL;
}

/* ADMIN FUNCTIONS */
void addSubject(){
    if(subjectCount>=MAX_SUBJECTS){ printf("Max subjects reached\n"); return;}
    printf("Subject name: "); readLine(subjects[subjectCount],NAME_LEN);
    subjectCount++;
}

void listSubjects(){
    if(subjectCount==0){ printf("No subjects\n"); return;}
    printf("\nSubjects:\n");
    for(int i=0;i<subjectCount;i++)
        printf("%d) %s\n", i, subjects[i]);
}

void deleteSubject(){
    if(subjectCount==0){ printf("No subjects to delete\n"); return;}
    listSubjects();
    char buf[16]; printf("Enter subject index to delete: "); readLine(buf,16);
    int idx=atoi(buf);
    if(idx<0 || idx>=subjectCount){ printf("Invalid index\n"); return;}
    for(int j=idx;j<subjectCount-1;j++)
        strcpy(subjects[j],subjects[j+1]);
    for(int i=0;i<studentCount;i++)
        for(int j=idx;j<subjectCount-1;j++)
            students[i].scores[j]=students[i].scores[j+1];
    subjectCount--;
    printf("Subject deleted\n");
}

void addStudent(){
    if(studentCount>=MAX_STUDENTS || accountCount>=MAX_ACCOUNTS){ printf("Max reached\n"); return;}
    Student s; s.id=nextStudentId++; s.reputation=3.0; s.ratingCount=0;
    memset(s.scores,0,sizeof(s.scores)); memset(s.freeTime,0,sizeof(s.freeTime));
    printf("Student name: "); readLine(s.name,NAME_LEN);
    students[studentCount++]=s;
    Account a; printf("Username: "); readLine(a.username,NAME_LEN);
    printf("Password: "); readLine(a.password,NAME_LEN);
    a.role=ROLE_STUDENT; a.studentId=s.id;
    accounts[accountCount++]=a;
    printf("Student created with ID %d\n", s.id);
}

void listStudentsAdmin(){
    if(studentCount==0){ printf("No students\n"); return;}
    printf("\nID   Name               Reputation\n");
    printf("-----------------------------------\n");
    for(int i=0;i<studentCount;i++)
        printf("%-4d %-18s %.2f\n", students[i].id, students[i].name, students[i].reputation);
}

void deleteStudent(){
    if(studentCount==0){ printf("No students to delete\n"); return;}
    listStudentsAdmin();
    char buf[16]; printf("Enter student ID to delete: "); readLine(buf,16);
    int id=atoi(buf);
    int idx=-1;
    for(int i=0;i<studentCount;i++) if(students[i].id==id){ idx=i; break;}
    if(idx==-1){ printf("Student not found\n"); return;}
    for(int i=idx;i<studentCount-1;i++) students[i]=students[i+1];
    studentCount--;
    for(int i=0;i<accountCount;i++)
        if(accounts[i].studentId==id){ for(int j=i;j<accountCount-1;j++) accounts[j]=accounts[j+1]; accountCount--; break;}
    printf("Student deleted\n");
}

void editStudentScoresAdmin(){
    if(studentCount==0 || subjectCount==0){ printf("No students or subjects\n"); return;}
    listStudentsAdmin();
    char buf[32]; int id; printf("Enter student ID to edit scores: "); readLine(buf,32); id=atoi(buf);
    Student *s=findStudent(id); if(!s){ printf("Student not found\n"); return;}
    for(int j=0;j<subjectCount;j++){
        printf("%s current score: %d -> new score (0-100, empty to keep): ", subjects[j], s->scores[j]);
        readLine(buf,32); if(strlen(buf)==0) continue;
        int v=atoi(buf); if(v<0)v=0; if(v>100)v=100; s->scores[j]=v;
    }
}

/* STUDENT FUNCTIONS */
void setAvailability(Student *s){
    int d,a,b; char buf[32];
    printf("Enter availability: day start end (0 0 0 to stop)\n");
    while(1){
        readLine(buf,32); sscanf(buf,"%d %d %d",&d,&a,&b);
        if(d==0) break;
        if(d<1||d>DAYS||a<0||a>=HOURS||b<=a||b>HOURS){ printf("Invalid\n"); continue;}
        for(int h=a;h<b;h++) s->freeTime[d-1][h]=1;
    }
}

int overlap(Student *a, Student *b){ int c=0; for(int d=0;d<DAYS;d++)
for(int h=0;h<HOURS;h++) if(a->freeTime[d][h] && b->freeTime[d][h])
    c++; return c;}

double matchScore(Student *me, Student *o, int subj){
    double s=0.0; if(subj<0 || subj>=MAX_SUBJECTS) return 0;
    if(me->scores[subj]<50 && o->scores[subj]>=70) s+=1.0;
    s += (o->reputation-1)/4.0;
    s += overlap(me,o)/40.0;
    return s;
}

void proposePartner(Student *me){
    if(subjectCount==0){
        printf("No subjects\n");
    return;
    }
    char buf[16]; int subj;
    listSubjects();
    printf("Choose subject index (0..%d): ",subjectCount-1);
    readLine(buf,16);
    subj=atoi(buf);
    double best=-1; Student *bestS=NULL;
    for(int i=0;i<studentCount;i++){
            Student *o=&students[i];
    if(o==me) continue; double sc=matchScore(me,o,subj);
    if(sc>best){ best=sc; bestS=o;}}
    if(bestS) printf("Suggested partner: %s ID %d score %.2f\n", bestS->name,bestS->id,best);
    else printf("No partner found\n");
}

void rateStudent(Student *me){
    char buf[16]; int id,r;
    printf("Partner ID: "); readLine(buf,16); id=atoi(buf);
    Student *o=findStudent(id); if(!o||o==me) return;
    printf("Rating 1 to 5: "); readLine(buf,16); r=atoi(buf); if(r<1||r>5) return;
    o->reputation=(o->reputation*o->ratingCount+r)/(o->ratingCount+1); o->ratingCount++;
}

void editMyScores(Student *s){
    if(subjectCount==0){ printf("No subjects\n"); return;}
    char buf[32]; for(int j=0;j<subjectCount;j++){
        printf("%s current score: %d -> new score (0-100, empty to keep): ", subjects[j], s->scores[j]);
        readLine(buf,32); if(strlen(buf)==0) continue;
        int v=atoi(buf); if(v<0)v=0;if(v>100)v=100; s->scores[j]=v;
    }
}

/* EXPORT HUMAN-READABLE DATA */
void exportDataHumanReadable(){
    FILE *f = fopen("data.txt","w");
    if(!f){ printf("Failed to open data.txt\n"); return; }

    fprintf(f, "===== SUBJECTS =====\n");
    for(int i=0;i<subjectCount;i++)
        fprintf(f,"%d: %s\n", i, subjects[i]);
    fprintf(f,"\n");

    fprintf(f, "===== STUDENTS =====\n");
    for(int i=0;i<studentCount;i++){
        Student *s = &students[i];
        fprintf(f,"ID: %d, Name: %s, Reputation: %.2f, Ratings: %d\n", s->id, s->name, s->reputation, s->ratingCount);
        fprintf(f,"Scores: ");
        for(int j=0;j<subjectCount;j++)
            fprintf(f,"%s=%d ", subjects[j], s->scores[j]);
        fprintf(f,"\nAvailability (DAYSxHOURS):\n");
        for(int d=0; d<DAYS; d++){
            fprintf(f,"Day %d: ", d+1);
            for(int h=0; h<HOURS; h++)
                fprintf(f,"%d", s->freeTime[d][h]);
            fprintf(f,"\n");
        }
        fprintf(f,"\n");
    }

    fprintf(f, "===== ACCOUNTS =====\n");
    for(int i=0;i<accountCount;i++){
        Account *a = &accounts[i];
        fprintf(f,"Username: %s, Role: %s", a->username, a->role==ROLE_ADMIN?"ADMIN":"STUDENT");
        if(a->role==ROLE_STUDENT) fprintf(f,", StudentID: %d", a->studentId);
        fprintf(f,"\n");
    }

    fclose(f);
    printf("Data exported to data.txt\n");
}

/* MENUS */
void adminMenu(){
    char c[8];
    while(1){
        printf("\nADMIN MENU\n1 Add subject\n2 List subjects\n3 Delete subject\n4 Create student account\n5 List students\n6 Delete student\n7 Edit student scores\n8 Save\n9 Export human-readable\n0 Logout\nChoice: ");
        readLine(c,8);
        if(c[0]=='1') addSubject();
        else if(c[0]=='2') listSubjects();
        else if(c[0]=='3') deleteSubject();
        else if(c[0]=='4') addStudent();
        else if(c[0]=='5') listStudentsAdmin();
        else if(c[0]=='6') deleteStudent();
        else if(c[0]=='7') editStudentScoresAdmin();
        else if(c[0]=='8') saveData();
        else if(c[0]=='9') exportDataHumanReadable();
        else if(c[0]=='0') break;
    }
}

void studentMenu(Account *acc){
    Student *me=findStudent(acc->studentId); if(!me) return;
    char c[8];
    while(1){
        printf("\nSTUDENT MENU %s\n1 Set availability\n2 List subjects\n3 Propose partner\n4 Rate student\n5 Save\n6 Edit my scores\n0 Logout\nChoice: ",me->name);
        readLine(c,8);
        if(c[0]=='1') setAvailability(me);
        else if(c[0]=='2') listSubjects();
        else if(c[0]=='3') proposePartner(me);
        else if(c[0]=='4') rateStudent(me);
        else if(c[0]=='5') saveData();
        else if(c[0]=='6') editMyScores(me);
        else if(c[0]=='0') break;
    }
}

/* MAIN */
int main(){
    loadData();
    if(accountCount==0){ Account admin={"admin","admin",ROLE_ADMIN,-1}; accounts[accountCount++]=admin; }
    while(1){
        printf("\nLOGIN\n"); Account *acc=login();
        if(!acc){ printf("Login failed\n"); continue;}
        if(acc->role==ROLE_ADMIN) adminMenu();
        else studentMenu(acc);
    }
    return 0;
}
