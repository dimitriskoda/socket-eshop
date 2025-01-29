Ηλεκτρονικό Κατάστημα με Sockets - Μία Εκτελέσιμη Μορφή

1) Περίληψη:
   Το πρόγραμμα myeshop μπορεί να τρέξει είτε ως server είτε ως client,
   αναλόγως με το όρισμα "server" ή "client".

2) Αρχεία:
   - myeshop.h / myeshop.c: Κώδικας προϊόντων, στοκ, συναρτήσεις orderHandler κλπ.
   - main.c: Περιέχει τις serverFunction() / clientFunction() και το main().
   - Makefile: Κανόνες μεταγλώττισης.

3) Μεταγλώττιση:
   make

4) Εκτέλεση:
   - Σε ένα τερματικό: ./myeshop server (ή ./myeshop server & για background)
   - Σε δεύτερο τερματικό: ./myeshop client
     (Δημιουργεί 5 child διεργασίες, στέλνουν 10 παραγγελίες η καθεμία.)
