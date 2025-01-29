#ifndef MYESHOP_H
#define MYESHOP_H

/* 
  myeshop.h

  Δηλώσεις σταθερών, δομών και συναρτήσεων για το e-shop:
  - PRODUCTS_COUNT: σύνολο προϊόντων
  - ProductStruct: περιγραφή προϊόντος
  - Καθολικές μεταβλητές όπως catalogArray
  - Συναρτήσεις: initializeCatalog() και orderHandler()
*/

#define PRODUCTS_COUNT 20

/* 
  ProductStruct:
    - desc:          όνομα/περιγραφή προϊόντος
    - price:         τιμή του
    - stock:         πόσα τεμάχια διαθέσιμα
    - reqCount:      πόσες φορές ζητήθηκε
    - soldCount:     πόσα τεμάχια πουλήθηκαν
    - failIDs[]:     ποιοι πελάτες απέτυχαν
    - failCnt:       πόσες αποτυχίες υπάρχουν
*/
typedef struct {
    char  desc[50];
    float price;
    int   stock;
    int   reqCount;
    int   soldCount;
    int   failIDs[100];  // αποθηκεύουμε τους clientID που απέτυχαν
    int   failCnt;
} ProductStruct;

/*  
  Global μεταβλητές:
    - catalogArray: πίνακας με όλα τα προϊόντα
    - requestsTotal: πόσες συνολικές παραγγελίες έγιναν
    - successTotal:  πόσες πέτυχαν
    - failedTotal:   πόσες απέτυχαν
    - revenueTotal:  σύνολο εσόδων
*/
extern ProductStruct catalogArray[PRODUCTS_COUNT];
extern int requestsTotal;
extern int successTotal;
extern int failedTotal;
extern float revenueTotal;

/*
  initializeCatalog():
  - Φορτώνει τον πίνακα με 20 προϊόντα, καθένα με stock=2,
    τιμή = 10 + index κ.λπ.
*/
void initializeCatalog(void);

/*
  orderHandler(productID, clientID, costOut):
  - Αυξάνει counters (reqCount, requestsTotal).
  - Αν υπάρχει stock > 0, γίνεται αγορά:
      * μειώνεται stock
      * αυξάνονται successTotal, soldCount, revenueTotal
      * επιστρέφεται 1 στο success 
  - Αν δεν υπάρχει stock:
      * μπαίνει clientID σε failIDs
      * αυξάνεται failedTotal
      * success=0
  - Το costOut γεμίζει με την τιμή του προϊόντος αν πέτυχε, αλλιώς 0.0.
*/
int orderHandler(int productID, int clientID, float *costOut);

#endif
