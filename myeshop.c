/*
  myeshop.c

  Υλοποίηση συναρτήσεων και καθολικών μεταβλητών του myeshop.h
  (initializeCatalog και orderHandler).
*/

#include "myeshop.h"
#include <stdio.h>   // για sprintf
#include <stdlib.h>  // για rand, srand
#include <string.h>  // για strcpy

ProductStruct catalogArray[PRODUCTS_COUNT];
int   requestsTotal  = 0;
int   successTotal   = 0;
int   failedTotal    = 0;
float revenueTotal   = 0.0f;

void initializeCatalog(void)
{
    for (int i = 0; i < PRODUCTS_COUNT; i++) {
        sprintf(catalogArray[i].desc, "Product%d", i);
        catalogArray[i].price    = 10.0f + i;  // π.χ. 10.0, 11.0, 12.0...
        catalogArray[i].stock    = 2;          // κάθε προϊόν αρχίζει με 2 τεμάχια
        catalogArray[i].reqCount = 0;
        catalogArray[i].soldCount= 0;
        catalogArray[i].failCnt  = 0;
    }
}

int orderHandler(int productID, int clientID, float *costOut)
{
    // Αν το productID είναι εκτός ορίων
    if (productID < 0 || productID >= PRODUCTS_COUNT) {
        *costOut = 0.0f;
        return 0;
    }

    // Αύξηση counters
    catalogArray[productID].reqCount++;
    requestsTotal++;

    // Έλεγχος στοκ
    if (catalogArray[productID].stock > 0) {
        // Επιτυχία
        catalogArray[productID].stock--;
        catalogArray[productID].soldCount++;
        float localPrice = catalogArray[productID].price;
        *costOut = localPrice;
        successTotal++;
        revenueTotal += localPrice;
        return 1;
    }
    else {
        // Αποτυχία
        *costOut = 0.0f;
        failedTotal++;

        int fc = catalogArray[productID].failCnt;
        catalogArray[productID].failIDs[fc] = clientID;
        catalogArray[productID].failCnt++;

        return 0;
    }
}
