# Agenda de contacte – versiune intermediara

**Stadiu: WIP (work in progress), nu este versiunea finala.**

Proiect realizat in C++20 pentru disciplina Metode avansate de programare.

## Date student

- Nume: Lăzărescu Alexandru-Marcel
- Grupa: 1.2
- Numar matricol: 715692
- Email: alexandru-marcel.lazarescu@student.upt.ro
- Tema: 1 – Agenda de contacte

## Progres existent in acest checkpoint

- Structura de date `Contact` si conversia in JSON.
- Clasa `Store` pentru pastrarea contactelor in memorie.
- Alocarea identificatorilor pentru contacte.
- Operatii de adaugare, cautare, stergere si actualizare la nivelul clasei `Store`.
- Filtrare dupa categorie si text si sortare alfabetica.
- Primele rute HTTP: `/health`, `/version`, `/`, `GET /contacts` si `POST /contacts`.
- Primele teste unitare pentru logica de baza.

## Ce este inca de lucru

- Validarea completa a tuturor campurilor introduse prin API.
- Rutele HTTP pentru consultarea unui contact dupa ID, editare si stergere.
- Ruta de statistici si comportamentul de resetare.
- Extinderea testelor pentru erori si cazuri limita.
- Verificarea compilarii si a rularii in Docker.
- Revizuirea documentatiei dupa stabilizarea API-ului.

## Tehnologii

C++20, cpp-httplib, nlohmann/json, CMake, doctest si Docker.

## Compilare si rulare

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/server
```

Serverul este configurat pentru portul `8080`. Dependentele sunt descarcate de CMake la configurare, deci este necesara conexiune la internet daca nu sunt deja in cache.

## Rute disponibile in acest stadiu

| Metoda | Ruta | Stare |
|---|---|---|
| GET | `/health` | Implementata |
| GET | `/version` | Implementata |
| GET | `/` | Implementata |
| GET | `/contacts` | Implementata – listare |
| POST | `/contacts` | Implementata – adaugare de baza |
| GET/PUT/DELETE | `/contacts/{id}` | In lucru |
| GET | `/stats` | In lucru |

## Note

Acesta este un instantaneu al stadiului curent de dezvoltare. Nu reprezinta versiunea finala si trebuie completat si testat inainte de predare.
