# SHA256
Il progetto consite nel realizzare un server che permetta multiple computazioni di impronte SHA-256. 

Il tempo di calcolo per impronta è proporzionale al numero di byte dell’ingresso (ovvero il file), dipendente dalla piattaforma e dalla implementazione dell’algoritmo (∼1 secondo per 70 MB su laptop moderno nella implementazione OpenSSL, ∼1 secondo per 30 KB su MentOS in QEMU). 

Successivamente va realizzato un client che invii l’informazione di file di input al server e riceva l’impronta risultante appena computata.

## 🛠️ Build e Run

### 1. Prerequisiti
Il sistema richiede le librerie di sviluppo **OpenSSL** per il calcolo crittografico e **CMake** per la compilazione.

* **macOS (via Homebrew):**
    ```bash
    brew install openssl cmake
    ```
* **Ubuntu/Debian:**
    ```bash
    sudo apt update
    sudo apt install libssl-dev cmake build-essential
    ```

### 2. Compilazione
Dalla cartella principale del progetto, eseguire i seguenti comandi per generare gli eseguibili.

    mkdir build
    cd build
    cmake ..
    make

### 3. Esecuzione
Il sistema richieda l'apertura di terminali separati.

* **Terminale 1: avvio del server**
    ```bash
    ./server
    ```
* **Terminale 2: invio richiesta**
    ```bash
    ./client <path/nome_file>
    ``` 

* **Terminale 3 (opzionale): query cache**
    ``` bash
    ./cache_query
    ```

### 4. Chiusura e Pulizia
* Per fermare il server: Premere `CTRL+C` nel terminale del server.
* Per pulire i file temporanei: `rm -rf build`