#include "wrapper.h"

#define ALLOCATED_FLAG 1
#define FREE_FLAG 0

/* Kod funkcii my_init, my_alloc a my_free nahradte vlastnym. Nepouzivajte ziadne
 * globalne ani staticke premenne; jedina globalna pamat je dostupna pomocou
 * mread/mwrite/msize, ktorych popis najdete vo wrapper.h */

/* Ukazkovy kod zvladne naraz iba jedinu alokaciu. V 0-tom bajte pamate si
 * pamata, ci je pamat od 1 dalej volna alebo obsadena.
 *
 * V pripade, ze je volna, volanie my_allloc skonci uspesne a vrati zaciatok
 * alokovanej RAM; my_free pri volnej mamati zlyha.
 *
 * Ak uz nejaka alokacia prebehla a v 0-tom bajte je nenulova hodnota. Nie je
 * mozne spravit dalsiu alokaciu, takze my_alloc musi zlyhat. my_free naopak
 * zbehnut moze a uvolni pamat.
 */



// Write an unsigned int size to memory
void write_size(unsigned int addr, unsigned int size) {
    // Write each byte of the unsigned int value to memory
    for (int i = 0; i < sizeof(unsigned int); i++) {
        mwrite(addr + i, (uint8_t)((size >> (i * 8)) & 0xFF));
    }
}

// Read an unsigned int size from memory
unsigned int read_size(unsigned int addr) {
    unsigned int size = 0;
    // Read each byte from memory and construct the unsigned int value
    for (int i = 0; i < sizeof(unsigned int); i++) {
        size |= ((unsigned int)mread(addr + i) & 0xFF) << (i * 8);
    }
    return size;
}

// Find free memory
unsigned int find_free_memory(unsigned int size) {
    unsigned int addr = 0;
    unsigned int count = 0;

    while (addr < msize()) {
        if (mread(addr) == FREE_FLAG) {
            count++;
            if (count == size + 5) {
                mwrite(addr - size - 4, ALLOCATED_FLAG);
                write_size(addr - size - 3, size);
                return addr - size + 1;
            }
            addr++;
        } else {
            count = 0;
            addr += read_size(addr+1) + 5;
        }
    }

    return FAIL;
}

/**
 * Inicializacia pamate
 *
 * Zavola sa, v stave, ked sa zacina s prazdnou pamatou, ktora je inicializovana
 * na 0.
 */
void my_init(void) {
    return;
}

/**
 * Poziadavka na alokaciu 'size' pamate.
 *
 * Ak sa pamat podari alokovat, navratova hodnota je adresou prveho bajtu
 * alokovaneho priestoru v RAM. Pokial pamat uz nie je mozne alokovat, funkcia
 * vracia FAIL.
 */
int my_alloc(unsigned int size) {

    /* Nemozeme alokovat viac pamate, ako je dostupne */
    if (size >= msize() - 5)
        return FAIL;

    unsigned int addr = find_free_memory(size);

    if (addr == FAIL) return FAIL;

    return addr;
}

/**
 * Poziadavka na uvolnenie alokovanej pamate na adrese 'addr'.
 *
 * Ak bola pamat zacinajuca na adrese 'addr' alokovana, my_free ju uvolni a
 * vrati OK. Ak je adresa 'addr' chybna (nezacina na nej ziadna alokovana
 * pamat), my_free vracia FAIL.
 */

int my_free(unsigned int addr) {

    /* Is allocated? */
    if (addr < 5 || addr >= msize()) return FAIL;
    if (mread(addr - 5) != ALLOCATED_FLAG)
        return FAIL;



    // Free
    unsigned int a = 0;
    while (a + 5 < addr) {
        if (mread(a) == ALLOCATED_FLAG) {
            a++;
            a += 4 + read_size(a);
        } else a++;
    } if (a + 5 != addr)
        return FAIL;

    if (mread(a) != ALLOCATED_FLAG) return FAIL;

    unsigned int size = read_size(addr-4);
    for (unsigned int i = addr-5; i < addr + size; i++) {
        mwrite(i, FREE_FLAG);

    }

    return OK;
}

