/*
    Campus Food Delivery Management System (CFdMs) - Single File Version
    Modules: Student, Restaurant, Delivery, Admin
    Persistence: CSV text files
    NOTE: Keep names without commas to keep CSV parsing simple.
    Build: gcc cfdms.c -o cfdms
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FILE_STUDENTS   "students.csv"    // id,name,email,password,balance,active
#define FILE_RESTS      "restaurants.csv" // id,name,email,password,active
#define FILE_DELS       "delivery.csv"    // id,name,phone,password,earnings,active
#define FILE_MENU       "menu.csv"        // restId,itemId,itemName,price,stock
#define FILE_ORDERS     "orders.csv"      // orderId,studentId,restId,itemId,qty,total,status,deliveryId,iso8601

#define MAX 128
#define MAX_NAME 64
#define MAX_EMAIL 64
#define MAX_PASS 32
#define MAX_PHONE 20
#define ADMIN_USER "admin"
#define ADMIN_PASS "admin123" // change if you want

/* ======================== Data Models ======================== */

typedef struct {
    char id[16];
    char name[MAX_NAME];
    char email[MAX_EMAIL];
    char password[MAX_PASS];
    double balance;
    int active; // 1 active, 0 deactivated
} Student;

typedef struct {
    char id[16];
    char name[MAX_NAME];
    char email[MAX_EMAIL];
    char password[MAX_PASS];
    int active;
} Restaurant;

typedef struct {
    char id[16];
    char name[MAX_NAME];
    char phone[MAX_PHONE];
    char password[MAX_PASS];
    double earnings;
    int active;
} Delivery;

typedef struct {
    char restId[16];
    int itemId;
    char itemName[MAX_NAME];
    double price;
    int stock;
} MenuItem;

typedef struct {
    int orderId;
    char studentId[16];
    char restId[16];
    int itemId;
    int qty;
    double total;
    char status[16];          // "Pending","Prepared","Assigned","Delivered","Cancelled"
    char deliveryId[16];      // "-" if none
    char isoTime[32];         // ISO 8601 timestamp
} Order;

/* ======================== Utilities ======================== */

void ensureFilesExist() {
    const char *files[] = { FILE_STUDENTS, FILE_RESTS, FILE_DELS, FILE_MENU, FILE_ORDERS };
    for (int i=0;i<5;i++){
        FILE *f = fopen(files[i], "a");
        if (f) fclose(f);
    }
}

void clearInput() {
    int c;
    while ((c=getchar())!='\n' && c!=EOF) {}
}

int readLine(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = 0;
    return 1;
}

int isValidEmail(const char *s) {
    return strchr(s,'@') && strchr(s,'.');
}

void pauseEnter() {
    printf("\nPress Enter to continue...");
    clearInput();
}

void nowIso(char *out, size_t n) {
    time_t t=time(NULL);
    struct tm *tmv = localtime(&t);
    strftime(out, n, "%Y-%m-%dT%H:%M:%S", tmv);
}

int yesNo(const char *q){
    char ans[8];
    printf("%s (y/n): ", q);
    readLine(ans, sizeof(ans));
    return tolower(ans[0])=='y';
}

/* ======================== CSV Helpers ======================== */

int parseStudent(const char *line, Student *s) {
    // id,name,email,password,balance,active
    char tmp[512]; strncpy(tmp,line,sizeof(tmp)); tmp[sizeof(tmp)-1]=0;
    char *tok = strtok(tmp,","); if(!tok) return 0; strncpy(s->id,tok,sizeof(s->id));
    tok = strtok(NULL,","); if(!tok) return 0; strncpy(s->name,tok,sizeof(s->name));
    tok = strtok(NULL,","); if(!tok) return 0; strncpy(s->email,tok,sizeof(s->email));
    tok = strtok(NULL,","); if(!tok) return 0; strncpy(s->password,tok,sizeof(s->password));
    tok = strtok(NULL,","); if(!tok) return 0; s->balance = atof(tok);
    tok = strtok(NULL,","); if(!tok) return 0; s->active = atoi(tok);
    return 1;
}

void printStudentCSV(FILE *f, const Student *s){
    fprintf(f,"%s,%s,%s,%s,%.2f,%d\n", s->id, s->name, s->email, s->password, s->balance, s->active);
}

int parseRestaurant(const char *line, Restaurant *r){
    // id,name,email,password,active
    char tmp[512]; strncpy(tmp,line,sizeof(tmp)); tmp[sizeof(tmp)-1]=0;
    char *tok=strtok(tmp,","); if(!tok) return 0; strncpy(r->id,tok,sizeof(r->id));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(r->name,tok,sizeof(r->name));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(r->email,tok,sizeof(r->email));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(r->password,tok,sizeof(r->password));
    tok=strtok(NULL,","); if(!tok) return 0; r->active=atoi(tok);
    return 1;
}

void printRestaurantCSV(FILE *f,const Restaurant *r){
    fprintf(f,"%s,%s,%s,%s,%d\n", r->id, r->name, r->email, r->password, r->active);
}

int parseDelivery(const char *line, Delivery *d){
    // id,name,phone,password,earnings,active
    char tmp[512]; strncpy(tmp,line,sizeof(tmp)); tmp[sizeof(tmp)-1]=0;
    char *tok=strtok(tmp,","); if(!tok) return 0; strncpy(d->id,tok,sizeof(d->id));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(d->name,tok,sizeof(d->name));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(d->phone,tok,sizeof(d->phone));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(d->password,tok,sizeof(d->password));
    tok=strtok(NULL,","); if(!tok) return 0; d->earnings=atof(tok);
    tok=strtok(NULL,","); if(!tok) return 0; d->active=atoi(tok);
    return 1;
}

void printDeliveryCSV(FILE *f, const Delivery *d){
    fprintf(f,"%s,%s,%s,%s,%.2f,%d\n", d->id, d->name, d->phone, d->password, d->earnings, d->active);
}

int parseMenu(const char *line, MenuItem *m){
    // restId,itemId,itemName,price,stock
    char tmp[512]; strncpy(tmp,line,sizeof(tmp)); tmp[sizeof(tmp)-1]=0;
    char *tok=strtok(tmp,","); if(!tok) return 0; strncpy(m->restId,tok,sizeof(m->restId));
    tok=strtok(NULL,","); if(!tok) return 0; m->itemId=atoi(tok);
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(m->itemName,tok,sizeof(m->itemName));
    tok=strtok(NULL,","); if(!tok) return 0; m->price=atof(tok);
    tok=strtok(NULL,","); if(!tok) return 0; m->stock=atoi(tok);
    return 1;
}

void printMenuCSV(FILE *f,const MenuItem *m){
    fprintf(f,"%s,%d,%s,%.2f,%d\n", m->restId, m->itemId, m->itemName, m->price, m->stock);
}

int parseOrder(const char *line, Order *o){
    // orderId,studentId,restId,itemId,qty,total,status,deliveryId,isoTime
    char tmp[512]; strncpy(tmp,line,sizeof(tmp)); tmp[sizeof(tmp)-1]=0;
    char *tok=strtok(tmp,","); if(!tok) return 0; o->orderId=atoi(tok);
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(o->studentId,tok,sizeof(o->studentId));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(o->restId,tok,sizeof(o->restId));
    tok=strtok(NULL,","); if(!tok) return 0; o->itemId=atoi(tok);
    tok=strtok(NULL,","); if(!tok) return 0; o->qty=atoi(tok);
    tok=strtok(NULL,","); if(!tok) return 0; o->total=atof(tok);
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(o->status,tok,sizeof(o->status));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(o->deliveryId,tok,sizeof(o->deliveryId));
    tok=strtok(NULL,","); if(!tok) return 0; strncpy(o->isoTime,tok,sizeof(o->isoTime));
    return 1;
}

void printOrderCSV(FILE *f,const Order *o){
    fprintf(f,"%d,%s,%s,%d,%d,%.2f,%s,%s,%s\n",
        o->orderId,o->studentId,o->restId,o->itemId,o->qty,o->total,o->status,o->deliveryId,o->isoTime);
}

/* ======================== Common Lookups ======================== */

int idExistsInFile(const char *file,const char *id,int which){ // which: 0=Student 1=Rest 2=Del
    FILE *fp=fopen(file,"r"); if(!fp) return 0;
    char line[512];
    if(which==0){
        Student s; while(fgets(line,sizeof(line),fp)) if(parseStudent(line,&s)&&strcmp(s.id,id)==0){fclose(fp);return 1;}
    } else if(which==1){
        Restaurant r; while(fgets(line,sizeof(line),fp)) if(parseRestaurant(line,&r)&&strcmp(r.id,id)==0){fclose(fp);return 1;}
    } else {
        Delivery d; while(fgets(line,sizeof(line),fp)) if(parseDelivery(line,&d)&&strcmp(d.id,id)==0){fclose(fp);return 1;}
    }
    fclose(fp); return 0;
}

int generateNextOrderId(){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp) return 1;
    char line[512]; int maxId=0; Order o;
    while(fgets(line,sizeof(line),fp)) if(parseOrder(line,&o) && o.orderId>maxId) maxId=o.orderId;
    fclose(fp);
    return maxId+1;
}

/* ======================== Student Module ======================== */

void studentRegister(){
    Student s={0};
    printf("\n--- Student Registration ---\n");
    printf("ID (e.g., TPxxxxxx): "); readLine(s.id,sizeof(s.id));
    if(idExistsInFile(FILE_STUDENTS,s.id,0)){ printf("ID already exists.\n"); return; }
    printf("Name (no commas): "); readLine(s.name,sizeof(s.name));
    do {
        printf("Email: "); readLine(s.email,sizeof(s.email));
        if(!isValidEmail(s.email)) printf("Invalid email format.\n");
    } while(!isValidEmail(s.email));
    printf("Password: "); readLine(s.password,sizeof(s.password));
    s.balance=0.0; s.active=1;
    FILE *fp=fopen(FILE_STUDENTS,"a"); if(!fp){ printf("File error.\n"); return; }
    printStudentCSV(fp,&s); fclose(fp);
    printf("Student registered.\n");
}

int studentLogin(Student *out){
    char id[16], pass[MAX_PASS];
    printf("\n--- Student Login ---\n");
    printf("ID: "); readLine(id,sizeof(id));
    printf("Password: "); readLine(pass,sizeof(pass));
    FILE *fp=fopen(FILE_STUDENTS,"r"); if(!fp){ printf("No students yet.\n"); return 0; }
    char line[512]; Student s;
    while(fgets(line,sizeof(line),fp)){
        if(parseStudent(line,&s) && strcmp(s.id,id)==0 && strcmp(s.password,pass)==0){
            fclose(fp);
            if(!s.active){ printf("Account is deactivated. Contact admin.\n"); return 0; }
            *out=s; return 1;
        }
    }
    fclose(fp);
    printf("Invalid credentials.\n");
    return 0;
}

void persistStudent(const Student *s){
    // rewrite file replacing this id
    FILE *fp=fopen(FILE_STUDENTS,"r"); if(!fp){ printf("File error.\n"); return; }
    FILE *tmp=fopen("students.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; Student cur;
    while(fgets(line,sizeof(line),fp)){
        if(parseStudent(line,&cur)){
            if(strcmp(cur.id,s->id)==0) printStudentCSV(tmp,s);
            else printStudentCSV(tmp,&cur);
        }
    }
    fclose(fp); fclose(tmp);
    remove(FILE_STUDENTS); rename("students.tmp",FILE_STUDENTS);
}

void studentView(const Student *s){
    printf("\n--- Profile ---\n");
    printf("ID: %s\nName: %s\nEmail: %s\nBalance: RM %.2f\nStatus: %s\n",
        s->id,s->name,s->email,s->balance,s->active?"Active":"Inactive");
}

void studentUpdate(Student *s){
    printf("\n1) Name  2) Email  3) Password\nChoose: ");
    char buf[16]; readLine(buf,sizeof(buf));
    int ch=atoi(buf);
    if(ch==1){ printf("New name: "); readLine(s->name,sizeof(s->name)); }
    else if(ch==2){ do{ printf("New email: "); readLine(s->email,sizeof(s->email)); }while(!isValidEmail(s->email)); }
    else if(ch==3){ printf("New password: "); readLine(s->password,sizeof(s->password)); }
    else { printf("Invalid.\n"); return; }
    persistStudent(s);
    printf("Updated.\n");
}

void studentTopUp(Student *s){
    printf("Amount to top-up RM: ");
    char buf[32]; readLine(buf,sizeof(buf));
    double amt=atof(buf);
    if(amt<=0){ printf("Invalid amount.\n"); return; }
    s->balance+=amt; persistStudent(s);
    printf("Top-up successful. New balance: RM %.2f\n", s->balance);
}

void listRestaurants(){
    FILE *fp=fopen(FILE_RESTS,"r"); if(!fp){ printf("No restaurants yet.\n"); return; }
    char line[512]; Restaurant r; printf("\n--- Restaurants ---\n");
    while(fgets(line,sizeof(line),fp)) if(parseRestaurant(line,&r)&&r.active) printf("%s - %s\n", r.id,r.name);
    fclose(fp);
}

void listMenuByRestaurant(const char *restId){
    FILE *fp=fopen(FILE_MENU,"r"); if(!fp){ printf("No menu yet.\n"); return; }
    char line[512]; MenuItem m; printf("\n--- Menu (%s) ---\n", restId);
    while(fgets(line,sizeof(line),fp)) if(parseMenu(line,&m) && strcmp(m.restId,restId)==0)
        printf("%d) %s  RM %.2f  [Stock:%d]\n", m.itemId, m.itemName, m.price, m.stock);
    fclose(fp);
}

int loadMenuItem(const char *restId,int itemId,MenuItem *out){
    FILE *fp=fopen(FILE_MENU,"r"); if(!fp) return 0;
    char line[512]; MenuItem m;
    while(fgets(line,sizeof(line),fp)) if(parseMenu(line,&m) && strcmp(m.restId,restId)==0 && m.itemId==itemId){
        *out=m; fclose(fp); return 1;
    }
    fclose(fp); return 0;
}

void updateMenuItem(const MenuItem *m){
    FILE *fp=fopen(FILE_MENU,"r"); if(!fp){ printf("Menu file error.\n"); return; }
    FILE *tmp=fopen("menu.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; MenuItem cur;
    while(fgets(line,sizeof(line),fp)){
        if(parseMenu(line,&cur)){
            if(strcmp(cur.restId,m->restId)==0 && cur.itemId==m->itemId) printMenuCSV(tmp,m);
            else printMenuCSV(tmp,&cur);
        }
    }
    fclose(fp); fclose(tmp);
    remove(FILE_MENU); rename("menu.tmp",FILE_MENU);
}

int studentPlaceOrder(Student *s){
    listRestaurants();
    char restId[16];
    printf("Choose Restaurant ID: "); readLine(restId,sizeof(restId));
    // verify restaurant exists & active
    FILE *fr=fopen(FILE_RESTS,"r"); if(!fr){ printf("No restaurants.\n"); return 0; }
    char line[512]; Restaurant r; int ok=0;
    while(fgets(line,sizeof(line),fr)) if(parseRestaurant(line,&r)&&strcmp(r.id,restId)==0 && r.active){ ok=1; break; }
    fclose(fr);
    if(!ok){ printf("Restaurant not found/active.\n"); return 0; }

    listMenuByRestaurant(restId);
    printf("Enter Item ID: "); char buf[32]; readLine(buf,sizeof(buf)); int itemId=atoi(buf);
    MenuItem mi;
    if(!loadMenuItem(restId,itemId,&mi)){ printf("Item not found.\n"); return 0; }
    printf("Qty: "); readLine(buf,sizeof(buf)); int qty=atoi(buf);
    if(qty<=0){ printf("Invalid qty.\n"); return 0; }
    if(qty>mi.stock){ printf("Insufficient stock.\n"); return 0; }

    double total = mi.price * qty;
    if(s->balance < total){ printf("Insufficient balance. Need RM %.2f\n", total - s->balance); return 0; }

    // Deduct student balance
    s->balance -= total; persistStudent(s);

    // Reduce stock
    mi.stock -= qty; updateMenuItem(&mi);

    // Create order
    Order o={0};
    o.orderId = generateNextOrderId();
    strncpy(o.studentId, s->id, sizeof(o.studentId));
    strncpy(o.restId, restId, sizeof(o.restId));
    o.itemId=itemId; o.qty=qty; o.total=total;
    strcpy(o.status,"Pending");
    strcpy(o.deliveryId,"-");
    nowIso(o.isoTime,sizeof(o.isoTime));

    FILE *fo=fopen(FILE_ORDERS,"a"); if(!fo){ printf("Order file error.\n"); return 0; }
    printOrderCSV(fo,&o); fclose(fo);

    printf("Order #%d placed. Total RM %.2f. Status: %s\n", o.orderId, o.total, o.status);
    return 1;
}

void studentOrderHistory(const Student *s){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders yet.\n"); return; }
    char line[512]; Order o; int any=0;
    printf("\n--- Your Orders ---\n");
    while(fgets(line,sizeof(line),fp)) if(parseOrder(line,&o) && strcmp(o.studentId,s->id)==0){
        printf("#%d  Rest:%s  Item:%d  Qty:%d  RM %.2f  Status:%s  Del:%s  %s\n",
            o.orderId,o.restId,o.itemId,o.qty,o.total,o.status,o.deliveryId,o.isoTime);
        any=1;
    }
    if(!any) printf("No orders.\n");
    fclose(fp);
}

void studentMenu(){
    Student s;
    int logged=studentLogin(&s);
    if(!logged) return;
    while(1){
        printf("\n--- Student Menu ---\n");
        printf("1.View Profile\n2.Update Profile\n3.Top-up\n4.Place Order\n5.View Order History\n0.Logout\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) studentView(&s);
        else if(ch==2){ studentUpdate(&s); }
        else if(ch==3){ studentTopUp(&s); }
        else if(ch==4){ studentPlaceOrder(&s); }
        else if(ch==5){ studentOrderHistory(&s); }
        else if(ch==0){ printf("Logged out.\n"); break; }
        else printf("Invalid.\n");
    }
}

/* ======================== Restaurant Module ======================== */

void restaurantRegister(){
    Restaurant r={0};
    printf("\n--- Restaurant Registration ---\n");
    printf("Restaurant ID (e.g., R001): "); readLine(r.id,sizeof(r.id));
    if(idExistsInFile(FILE_RESTS,r.id,1)){ printf("ID exists.\n"); return; }
    printf("Name (no commas): "); readLine(r.name,sizeof(r.name));
    do { printf("Email: "); readLine(r.email,sizeof(r.email));
         if(!isValidEmail(r.email)) printf("Invalid email.\n"); } while(!isValidEmail(r.email));
    printf("Password: "); readLine(r.password,sizeof(r.password));
    r.active=1;
    FILE *fp=fopen(FILE_RESTS,"a"); if(!fp){ printf("File error.\n"); return; }
    printRestaurantCSV(fp,&r); fclose(fp);
    printf("Restaurant registered.\n");
}

int restaurantLogin(Restaurant *out){
    char id[16], pass[MAX_PASS];
    printf("\n--- Restaurant Login ---\nID: "); readLine(id,sizeof(id));
    printf("Password: "); readLine(pass,sizeof(pass));
    FILE *fp=fopen(FILE_RESTS,"r"); if(!fp){ printf("No restaurants.\n"); return 0; }
    char line[512]; Restaurant r;
    while(fgets(line,sizeof(line),fp)){
        if(parseRestaurant(line,&r)&&strcmp(r.id,id)==0&&strcmp(r.password,pass)==0){
            fclose(fp);
            if(!r.active){ printf("Account deactivated.\n"); return 0; }
            *out=r; return 1;
        }
    }
    fclose(fp); printf("Invalid.\n"); return 0;
}

void restaurantPersist(const Restaurant *r){
    FILE *fp=fopen(FILE_RESTS,"r"); if(!fp){ printf("File error.\n"); return;}
    FILE *tmp=fopen("rests.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; Restaurant cur;
    while(fgets(line,sizeof(line),fp)){
        if(parseRestaurant(line,&cur)){
            if(strcmp(cur.id,r->id)==0) printRestaurantCSV(tmp,r);
            else printRestaurantCSV(tmp,&cur);
        }
    }
    fclose(fp); fclose(tmp); remove(FILE_RESTS); rename("rests.tmp",FILE_RESTS);
}

int nextItemIdForRest(const char *restId){
    FILE *fp=fopen(FILE_MENU,"r"); if(!fp) return 1;
    char line[512]; MenuItem m; int maxId=0;
    while(fgets(line,sizeof(line),fp)) if(parseMenu(line,&m) && strcmp(m.restId,restId)==0 && m.itemId>maxId) maxId=m.itemId;
    fclose(fp); return maxId+1;
}

void restMenuAdd(const char *restId){
    MenuItem m={0}; strncpy(m.restId,restId,sizeof(m.restId));
    m.itemId=nextItemIdForRest(restId);
    printf("Item name (no commas): "); readLine(m.itemName,sizeof(m.itemName));
    printf("Price: "); char buf[32]; readLine(buf,sizeof(buf)); m.price=atof(buf);
    printf("Initial stock: "); readLine(buf,sizeof(buf)); m.stock=atoi(buf);
    FILE *fp=fopen(FILE_MENU,"a"); if(!fp){ printf("File error.\n"); return; }
    printMenuCSV(fp,&m); fclose(fp);
    printf("Added item #%d.\n", m.itemId);
}

void restMenuUpdate(const char *restId){
    printf("Item ID to update: "); char buf[16]; readLine(buf,sizeof(buf)); int id=atoi(buf);
    MenuItem m; if(!loadMenuItem(restId,id,&m)){ printf("Not found.\n"); return; }
    printf("1.Name 2.Price 3.Stock  Choose: "); readLine(buf,sizeof(buf)); int ch=atoi(buf);
    if(ch==1){ printf("New name: "); readLine(m.itemName,sizeof(m.itemName)); }
    else if(ch==2){ printf("New price: "); readLine(buf,sizeof(buf)); m.price=atof(buf); }
    else if(ch==3){ printf("New stock: "); readLine(buf,sizeof(buf)); m.stock=atoi(buf); }
    else { printf("Invalid.\n"); return; }
    updateMenuItem(&m); printf("Updated.\n");
}

void restMenuDelete(const char *restId){
    printf("Item ID to delete: "); char buf[16]; readLine(buf,sizeof(buf)); int id=atoi(buf);
    FILE *fp=fopen(FILE_MENU,"r"); if(!fp){ printf("File error.\n"); return; }
    FILE *tmp=fopen("menu.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; MenuItem m; int removed=0;
    while(fgets(line,sizeof(line),fp)){
        if(parseMenu(line,&m)){
            if(strcmp(m.restId,restId)==0 && m.itemId==id){ removed=1; continue; }
            printMenuCSV(tmp,&m);
        }
    }
    fclose(fp); fclose(tmp); remove(FILE_MENU); rename("menu.tmp",FILE_MENU);
    printf(removed?"Deleted.\n":"Item not found.\n");
}

void restViewOrders(const char *restId, const char *statusFilter){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int any=0;
    printf("\n--- Orders (%s) ---\n", statusFilter?statusFilter:"All");
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && strcmp(o.restId,restId)==0){
            if(statusFilter==NULL || strcmp(o.status,statusFilter)==0){
                printf("#%d  Stud:%s  Item:%d  Qty:%d  RM %.2f  Status:%s  Del:%s  %s\n",
                    o.orderId,o.studentId,o.itemId,o.qty,o.total,o.status,o.deliveryId,o.isoTime);
                any=1;
            }
        }
    }
    if(!any) printf("None.\n");
    fclose(fp);
}

void persistOrder(const Order *o){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("Order file error.\n"); return; }
    FILE *tmp=fopen("orders.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; Order cur;
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&cur)){
            if(cur.orderId==o->orderId) printOrderCSV(tmp,o);
            else printOrderCSV(tmp,&cur);
        }
    }
    fclose(fp); fclose(tmp); remove(FILE_ORDERS); rename("orders.tmp",FILE_ORDERS);
}

void restMarkPrepared(const char *restId){
    printf("Order ID to mark Prepared: "); char buf[16]; readLine(buf,sizeof(buf)); int oid=atoi(buf);
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int found=0;
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && o.orderId==oid && strcmp(o.restId,restId)==0){
            found=1; break;
        }
    }
    fclose(fp);
    if(!found){ printf("Order not found.\n"); return; }
    if(strcmp(o.status,"Pending")!=0){ printf("Only Pending can be Prepared.\n"); return; }
    strcpy(o.status,"Prepared"); persistOrder(&o);
    printf("Order #%d marked Prepared.\n", oid);
}

void restDailySales(const char *restId){
    // Simple: sum ALL delivered totals for this restaurant (date filter can be added)
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; double sum=0; int cnt=0;
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && strcmp(o.restId,restId)==0 && strcmp(o.status,"Delivered")==0){
            sum+=o.total; cnt++;
        }
    }
    fclose(fp);
    printf("Delivered orders: %d   Total Sales: RM %.2f\n", cnt, sum);
}

void restaurantMenu(){
    Restaurant r;
    int logged=restaurantLogin(&r);
    if(!logged) return;
    while(1){
        printf("\n--- Restaurant Menu (%s) ---\n", r.name);
        printf("1.View Menu\n2.Add Item\n3.Update Item\n4.Delete Item\n5.View Pending Orders\n6.Mark Order Prepared\n7.View Delivered Sales\n0.Logout\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) listMenuByRestaurant(r.id);
        else if(ch==2) restMenuAdd(r.id);
        else if(ch==3) restMenuUpdate(r.id);
        else if(ch==4) restMenuDelete(r.id);
        else if(ch==5) restViewOrders(r.id,"Pending");
        else if(ch==6) restMarkPrepared(r.id);
        else if(ch==7) restDailySales(r.id);
        else if(ch==0) { printf("Logged out.\n"); break; }
        else printf("Invalid.\n");
    }
}

/* ======================== Delivery Module ======================== */

void deliveryRegister(){
    Delivery d={0};
    printf("\n--- Delivery Registration ---\n");
    printf("Delivery ID (e.g., D001): "); readLine(d.id,sizeof(d.id));
    if(idExistsInFile(FILE_DELS,d.id,2)){ printf("ID exists.\n"); return; }
    printf("Name (no commas): "); readLine(d.name,sizeof(d.name));
    printf("Phone: "); readLine(d.phone,sizeof(d.phone));
    printf("Password: "); readLine(d.password,sizeof(d.password));
    d.earnings=0.0; d.active=1;
    FILE *fp=fopen(FILE_DELS,"a"); if(!fp){ printf("File error.\n"); return; }
    printDeliveryCSV(fp,&d); fclose(fp);
    printf("Delivery personnel registered.\n");
}

int deliveryLogin(Delivery *out){
    char id[16], pass[MAX_PASS];
    printf("\n--- Delivery Login ---\nID: "); readLine(id,sizeof(id));
    printf("Password: "); readLine(pass,sizeof(pass));
    FILE *fp=fopen(FILE_DELS,"r"); if(!fp){ printf("No delivery personnel.\n"); return 0; }
    char line[512]; Delivery d;
    while(fgets(line,sizeof(line),fp)){
        if(parseDelivery(line,&d) && strcmp(d.id,id)==0 && strcmp(d.password,pass)==0){
            fclose(fp);
            if(!d.active){ printf("Account deactivated.\n"); return 0; }
            *out=d; return 1;
        }
    }
    fclose(fp); printf("Invalid.\n"); return 0;
}

void persistDelivery(const Delivery *d){
    FILE *fp=fopen(FILE_DELS,"r"); if(!fp){ printf("File error.\n"); return;}
    FILE *tmp=fopen("dels.tmp","w"); if(!tmp){ fclose(fp); return; }
    char line[512]; Delivery cur;
    while(fgets(line,sizeof(line),fp)){
        if(parseDelivery(line,&cur)){
            if(strcmp(cur.id,d->id)==0) printDeliveryCSV(tmp,d);
            else printDeliveryCSV(tmp,&cur);
        }
    }
    fclose(fp); fclose(tmp); remove(FILE_DELS); rename("dels.tmp",FILE_DELS);
}

void delListPreparedUnassigned(){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int any=0;
    printf("\n--- Prepared & Unassigned Orders ---\n");
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && strcmp(o.status,"Prepared")==0 && strcmp(o.deliveryId,"-")==0){
            printf("#%d  Rest:%s  Item:%d  Qty:%d  RM %.2f  %s\n", o.orderId,o.restId,o.itemId,o.qty,o.total,o.isoTime);
            any=1;
        }
    }
    if(!any) printf("None.\n");
    fclose(fp);
}

void delAcceptOrder(Delivery *d){
    printf("Order ID to accept: "); char buf[16]; readLine(buf,sizeof(buf)); int oid=atoi(buf);
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int found=0;
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && o.orderId==oid){ found=1; break; }
    }
    fclose(fp);
    if(!found){ printf("Order not found.\n"); return; }
    if(strcmp(o.status,"Prepared")!=0 || strcmp(o.deliveryId,"-")!=0){
        printf("Order not available for assignment.\n"); return;
    }
    strcpy(o.status,"Assigned"); strncpy(o.deliveryId,d->id,sizeof(o.deliveryId));
    persistOrder(&o);
    printf("Order #%d assigned to you.\n", oid);
}

void delMyOrders(const Delivery *d, const char *status){
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int any=0;
    printf("\n--- My Orders (%s) ---\n", status?status:"All");
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && strcmp(o.deliveryId,d->id)==0){
            if(status==NULL || strcmp(o.status,status)==0){
                printf("#%d  Rest:%s  Item:%d  Qty:%d  RM %.2f  Status:%s  %s\n",
                    o.orderId,o.restId,o.itemId,o.qty,o.total,o.status,o.isoTime);
                any=1;
            }
        }
    }
    if(!any) printf("None.\n");
    fclose(fp);
}

void delMarkDelivered(Delivery *d){
    printf("Order ID to mark Delivered: "); char buf[16]; readLine(buf,sizeof(buf)); int oid=atoi(buf);
    FILE *fp=fopen(FILE_ORDERS,"r"); if(!fp){ printf("No orders.\n"); return; }
    char line[512]; Order o; int found=0;
    while(fgets(line,sizeof(line),fp)){
        if(parseOrder(line,&o) && o.orderId==oid){ found=1; break; }
    }
    fclose(fp);
    if(!found){ printf("Order not found.\n"); return; }
    if(strcmp(o.deliveryId,d->id)!=0){ printf("Not your order.\n"); return; }
    if(strcmp(o.status,"Assigned")!=0){ printf("Only Assigned can be Delivered.\n"); return; }
    strcpy(o.status,"Delivered"); persistOrder(&o);
    // Earnings: simple flat commission 10% of order total
    d->earnings += o.total * 0.10;
    persistDelivery(d);
    printf("Delivered. Earnings now RM %.2f\n", d->earnings);
}

void deliveryMenu(){
    Delivery d;
    int logged=deliveryLogin(&d);
    if(!logged) return;
    while(1){
        printf("\n--- Delivery Menu (%s) ---\n", d.name);
        printf("1.View Prepared & Unassigned Orders\n2.Accept Order\n3.View My Assigned\n4.Mark Delivered\n5.View Earnings\n0.Logout\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) delListPreparedUnassigned();
        else if(ch==2) delAcceptOrder(&d);
        else if(ch==3) delMyOrders(&d,"Assigned");
        else if(ch==4) delMarkDelivered(&d);
        else if(ch==5) printf("Earnings: RM %.2f\n", d.earnings);
        else if(ch==0){ printf("Logged out.\n"); break; }
        else printf("Invalid.\n");
    }
}

/* ======================== Admin Module ======================== */

int adminLogin(){
    char u[32], p[32];
    printf("\n--- Admin Login ---\nUsername: "); readLine(u,sizeof(u));
    printf("Password: "); readLine(p,sizeof(p));
    if(strcmp(u,ADMIN_USER)==0 && strcmp(p,ADMIN_PASS)==0) return 1;
    printf("Invalid admin credentials.\n"); return 0;
}

void adminListStudents(){
    FILE *fp=fopen(FILE_STUDENTS,"r"); if(!fp){ printf("No students.\n"); return; }
    char line[512]; Student s; printf("\n--- Students ---\n");
    while(fgets(line,sizeof(line),fp)) if(parseStudent(line,&s))
        printf("%s  %-18s  Bal:RM%-8.2f  %s\n", s.id,s.name,s.balance,s.active?"Active":"Inactive");
    fclose(fp);
}

void adminListRestaurants(){
    FILE *fp=fopen(FILE_RESTS,"r"); if(!fp){ printf("No restaurants.\n"); return; }
    char line[512]; Restaurant r; printf("\n--- Restaurants ---\n");
    while(fgets(line,sizeof(line),fp)) if(parseRestaurant(line,&r))
        printf("%s  %-18s  %s\n", r.id,r.name,r.active?"Active":"Inactive");
    fclose(fp);
}

void adminListDelivery(){
    FILE *fp=fopen(FILE_DELS,"r"); if(!fp){ printf("No delivery.\n"); return; }
    char line[512]; Delivery d; printf("\n--- Delivery Personnel ---\n");
    while(fgets(line,sizeof(line),fp)) if(parseDelivery(line,&d))
        printf("%s  %-18s  Earn:RM%-8.2f  %s\n", d.id,d.name,d.earnings,d.active?"Active":"Inactive");
    fclose(fp);
}

void adminToggleUser(){
    printf("Toggle which type? 1.Student  2.Restaurant  3.Delivery : ");
    char buf[16]; readLine(buf,sizeof(buf)); int t=atoi(buf);
    char id[16]; printf("Enter ID: "); readLine(id,sizeof(id));
    if(t==1){
        FILE *fp=fopen(FILE_STUDENTS,"r"); if(!fp){ printf("File error.\n"); return; }
        FILE *tmp=fopen("students.tmp","w"); if(!tmp){ fclose(fp); return; }
        char line[512]; Student s; int changed=0;
        while(fgets(line,sizeof(line),fp)){
            if(parseStudent(line,&s)){
                if(strcmp(s.id,id)==0){ s.active=!s.active; changed=1; }
                printStudentCSV(tmp,&s);
            }
        }
        fclose(fp); fclose(tmp); remove(FILE_STUDENTS); rename("students.tmp",FILE_STUDENTS);
        printf(changed?"Toggled.\n":"ID not found.\n");
    } else if(t==2){
        FILE *fp=fopen(FILE_RESTS,"r"); if(!fp){ printf("File error.\n"); return; }
        FILE *tmp=fopen("rests.tmp","w"); if(!tmp){ fclose(fp); return; }
        char line[512]; Restaurant r; int changed=0;
        while(fgets(line,sizeof(line),fp)){
            if(parseRestaurant(line,&r)){
                if(strcmp(r.id,id)==0){ r.active=!r.active; changed=1; }
                printRestaurantCSV(tmp,&r);
            }
        }
        fclose(fp); fclose(tmp); remove(FILE_RESTS); rename("rests.tmp",FILE_RESTS);
        printf(changed?"Toggled.\n":"ID not found.\n");
    } else if(t==3){
        FILE *fp=fopen(FILE_DELS,"r"); if(!fp){ printf("File error.\n"); return; }
        FILE *tmp=fopen("dels.tmp","w"); if(!tmp){ fclose(fp); return; }
        char line[512]; Delivery d; int changed=0;
        while(fgets(line,sizeof(line),fp)){
            if(parseDelivery(line,&d)){
                if(strcmp(d.id,id)==0){ d.active=!d.active; changed=1; }
                printDeliveryCSV(tmp,&d);
            }
        }
        fclose(fp); fclose(tmp); remove(FILE_DELS); rename("dels.tmp",FILE_DELS);
        printf(changed?"Toggled.\n":"ID not found.\n");
    } else printf("Invalid type.\n");
}

void adminReports(){
    // Basic system-wide summary
    int stud=0,rest=0,del=0,orders=0,delivered=0; double sales=0, earnings=0;
    char line[512]; Student s; Restaurant r; Delivery d; Order o;

    FILE *fs=fopen(FILE_STUDENTS,"r"); if(fs){ while(fgets(line,sizeof(line),fs)) if(parseStudent(line,&s)) stud++; fclose(fs); }
    FILE *fr=fopen(FILE_RESTS,"r"); if(fr){ while(fgets(line,sizeof(line),fr)) if(parseRestaurant(line,&r)) rest++; fclose(fr); }
    FILE *fd=fopen(FILE_DELS,"r"); if(fd){ while(fgets(line,sizeof(line),fd)) if(parseDelivery(line,&d)){ del++; earnings+=d.earnings; } fclose(fd); }
    FILE *fo=fopen(FILE_ORDERS,"r"); if(fo){ while(fgets(line,sizeof(line),fo)) if(parseOrder(line,&o)){ orders++; if(strcmp(o.status,"Delivered")==0){ delivered++; sales+=o.total; } } fclose(fo); }

    printf("\n--- System Report ---\n");
    printf("Students: %d\nRestaurants: %d\nDelivery personnel: %d\n", stud,rest,del);
    printf("Orders total: %d  Delivered: %d\n", orders, delivered);
    printf("Total Sales (Delivered): RM %.2f\n", sales);
    printf("Total Delivery Earnings (accumulated): RM %.2f\n", earnings);
}

void adminMenu(){
    if(!adminLogin()) return;
    while(1){
        printf("\n--- Admin Menu ---\n");
        printf("1.View Students\n2.View Restaurants\n3.View Delivery\n4.Toggle Activate/Deactivate\n5.System Report\n0.Logout\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) adminListStudents();
        else if(ch==2) adminListRestaurants();
        else if(ch==3) adminListDelivery();
        else if(ch==4) adminToggleUser();
        else if(ch==5) adminReports();
        else if(ch==0){ printf("Admin logout.\n"); break; }
        else printf("Invalid.\n");
    }
}

/* ======================== Entry Menus & Main ======================== */

void studentEntry(){
    while(1){
        printf("\n--- Student Portal ---\n");
        printf("1.Register\n2.Login\n0.Back\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) studentRegister();
        else if(ch==2) studentMenu();
        else if(ch==0) break;
        else printf("Invalid.\n");
    }
}

void restaurantEntry(){
    while(1){
        printf("\n--- Restaurant Portal ---\n");
        printf("1.Register\n2.Login\n0.Back\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) restaurantRegister();
        else if(ch==2) restaurantMenu();
        else if(ch==0) break;
        else printf("Invalid.\n");
    }
}

void deliveryEntry(){
    while(1){
        printf("\n--- Delivery Portal ---\n");
        printf("1.Register\n2.Login\n0.Back\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) deliveryRegister();
        else if(ch==2) deliveryMenu();
        else if(ch==0) break;
        else printf("Invalid.\n");
    }
}

int main(){
    ensureFilesExist();
    while(1){
        printf("\n===== Campus Food Delivery Management System =====\n");
        printf("1. Student\n2. Restaurant\n3. Delivery\n4. Admin\n0. Exit\nChoose: ");
        char buf[16]; readLine(buf,sizeof(buf)); int ch=atoi(buf);
        if(ch==1) studentEntry();
        else if(ch==2) restaurantEntry();
        else if(ch==3) deliveryEntry();
        else if(ch==4) adminMenu();
        else if(ch==0){ printf("Goodbye!\n"); break; }
        else printf("Invalid choice.\n");
    }
    return 0;
}
