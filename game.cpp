#include <iostream>
#include <cmath>
#include <cassert>
#include <windows.h>
#include <cstring>
// PLOCHKI = CARTI
// Konstanti
const unsigned LAYER_SIZE = 20;
const unsigned DECK_SIZE = 8;
const unsigned INPUT_LENGTH = 64;
const unsigned MAX_LAYERS = 10;
unsigned deck_top = 0;

void print_layer(unsigned layer[][LAYER_SIZE], char symbols[]); // Printira sloi
void print_deck(unsigned deck[], char symbols[]); // Printira testeto
void setup_layer(unsigned layer[][LAYER_SIZE], size_t card_amount, unsigned id); // Suzdava sloi
bool update_deck(unsigned deck[], size_t card_amount); // Obnovqva testeto
void cleanup_deck(unsigned deck[], unsigned id); // Chisti povtorkite
void order_deck(unsigned deck[]); // Premestva vsichki karti vlqvo
bool check_coords(char input[], unsigned& x, unsigned& y); // Proverqva vhoda za podadeni koordinati
void take_card(unsigned layer[][LAYER_SIZE], unsigned deck[], unsigned x, unsigned y); // Vzema karta, podadena ot igracha i q slaga v testeto
void swap(unsigned& a, unsigned& b); // swap
void to_lower(char line[]); // Pravi vsichki glavni v malki bukvi
void clear(); // Chisti konzolata
bool check_win(unsigned layer[][LAYER_SIZE]); // Proverqva za pobeda
void setup_game(unsigned current_layer[][LAYER_SIZE][LAYER_SIZE], unsigned CARDS, unsigned LAYER_AMOUNT, unsigned card_amounts[]); // Suzdava neshtata za igrata
void flush_deck(unsigned deck[]); // Chisti testeto
// TODO
void overlay_layers(unsigned layers[][LAYER_SIZE][LAYER_SIZE], unsigned current_layer[][LAYER_SIZE]); // Nanasq sloevete edin nad drug

int main() {
    std::cout<<std::boolalpha;

    // Broi sloeve
    std::cout<<"How many layers do you want to have? [1, 10]"<<std::endl;
    int LAYER_AMOUNT;

    do {
        std::cout<<"> ";
        std::cin>>LAYER_AMOUNT;
    } while(LAYER_AMOUNT > 0 && LAYER_AMOUNT <= MAX_LAYERS);

    clear();

    // Vidove karti
    std::cout<<"How many different cards do you want to have? [8, 20]"<<std::endl;
    int CARDS;
    do {
        std::cout<<"> ";
        std::cin>>CARDS;
    } while(CARDS < 8 || CARDS > 20);

    clear();

    // Promenlivi
    char *card_symbols = new char[CARDS];
    unsigned *card_amounts = new unsigned[CARDS];
    unsigned (*layers)[LAYER_SIZE][LAYER_SIZE] = new unsigned[LAYER_AMOUNT][LAYER_SIZE][LAYER_SIZE];
    unsigned current_layer[LAYER_SIZE][LAYER_SIZE] = {};
    unsigned deck[DECK_SIZE] = {};

    // Vzemame simvoli, broq im i suzdavame poleto
    for(size_t i = 0; i < CARDS; i++) {
        std::cout<<"Enter symbol for card "<< i + 1 <<std::endl<<"> ";
        std::cin>>card_symbols[i];
        card_amounts[i] = (rand() % 7 + 1) * 3; // tova e za da si garantitame che nqma da stanat prekaleno mnogo kartite
    }

    clear();
    setup_game(layers, CARDS, LAYER_AMOUNT, card_amounts);

    char *input = new char[INPUT_LENGTH];

    // Game loop
    while(true) {
        // Pokazvame sloq i testeto
        print_layer(current_layer, card_symbols);
        print_deck(deck, card_symbols);

        // Priemame vhod
        std::cout<<"Insert command"<<std::endl<<"> ";
        std::cin.getline(input, INPUT_LENGTH);
        to_lower(input);

        // Tuk gledame vhoda ot protebitelq
        unsigned x, y;
        if(check_coords(input, x, y)) {
            take_card(current_layer, deck, x, y);
        } else {
            if(!strcmp(input, "kill")) {
                std::cout<<"Game has been halted";
                break;
            }
        }

        clear();

        // Pobeda
        if(check_win(current_layer)) {
            std::cout<<"YOU WIN!!!!";
            break;
        }

        // Zaguba
        if(update_deck(deck, CARDS)) {
            std::cout<<"YOU LOST"<<std::endl<<"Do you want to restart? [y/n]"<<std::endl<<"> ";
            char c;
            std::cin>>c;
            if(c == 'y') {
                setup_game(layers, CARDS, LAYER_AMOUNT, card_amounts);
                flush_deck(deck);
            } else {
                std::cout<<"Game has ended!"<<std::endl<<"BYE!"<<std::endl;
                break;
            }
        }
    }

    // Deletove
    delete[] input;
    delete[] card_amounts;
    delete[] card_symbols;
    delete[] layers;

    return 0;
}

void print_layer(unsigned layer[][LAYER_SIZE], char symbols[]) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout<<std::endl;
    for(size_t i = 0; i < LAYER_SIZE; i++) {
        std::cout<<'|';
        for(size_t j = 0; j < LAYER_SIZE; j++) {
            char symbol = ' ';
            if(layer[i][j] > 0) {
                symbol = symbols[layer[i][j] - 1];
                SetConsoleTextAttribute(h, layer[i][j] % 15);
            }
            std::cout<<symbol;
            SetConsoleTextAttribute(h, 15);
            std::cout<<'|';
        }
        std::cout<<std::endl;
    }
}

void print_deck(unsigned deck[], char symbols[]) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout<<std::endl;
    for(size_t i = 0; i < DECK_SIZE; i++) {
        if(deck[i] > 0) {
            SetConsoleTextAttribute(h, deck[i] % 15);
            std::cout<<symbols[deck[i] - 1];
            SetConsoleTextAttribute(h, 15);
        } else {
            std::cout<<'_';
        }

        std::cout<<' ';
    }

    std::cout<<std::endl;
}

void setup_layer(unsigned layer[][LAYER_SIZE], size_t card_amount, unsigned id) {
    for(size_t i = 0; i < card_amount; i++) {
        unsigned x, y;

        do{
            x = rand() % LAYER_SIZE;
            y = rand() % LAYER_SIZE;
        } while(layer[x][y]);

        layer[x][y] = id;
    }
}

bool update_deck(unsigned deck[], size_t card_amount) {
    unsigned cards[card_amount] = {};
    for(size_t i = 0; i < DECK_SIZE; i++) {
        if(deck[i] > 0) {
            unsigned index = deck[i] - 1;
            cards[index]++;
            if(cards[index] == 3) {
                cleanup_deck(deck, index + 1);
                i = 0;
            }
        }
    }

    return (deck[DECK_SIZE - 1] > 0);
}

void cleanup_deck(unsigned deck[], unsigned id) {
    for(size_t i = 0; i < DECK_SIZE; i++) {
        if(deck[i] == id) {
            deck[i] = 0;
        }
    }

    deck_top -= 3;

    order_deck(deck);
}

void order_deck(unsigned deck[]) {
    for(size_t i = 0; i < DECK_SIZE; i++) {
        if(i < DECK_SIZE - 1 && deck[i] == 0 && deck[i + 1] > 0) {
            swap(deck[i], deck[i + 1]);
        }
    }
}

void swap(unsigned& a, unsigned& b) {
    unsigned temp = a;
    a = b;
    b = temp;
}

void to_lower(char line[]) {
    char* ptr = line;
    while(*ptr) {
        if(*ptr >= 'A' && *ptr <= 'Z') {
            *ptr += 'a' - 'A';
        }
        ptr++;
    }
}

void clear() {
    system("cls");
}

bool check_coords(char input[], unsigned& x, unsigned& y) {
    char *ptr = input;
    bool has_x = false;
    while(*ptr) {
        if(*ptr >= '0' && *ptr <= '9') {
            if(has_x){
                x = atoi(ptr);
                return true;
            }
            else {
                y = atoi(ptr);
                has_x = true;
            }
        }
        ptr++;
    }

    return false;
}

void take_card(unsigned layer[][LAYER_SIZE], unsigned deck[], unsigned x, unsigned y) {
    if(layer[x][y] > 0) {
        deck[deck_top] = layer[x][y];
        layer[x][y] = 0;
        deck_top++;
    }
}

bool check_win(unsigned layer[][LAYER_SIZE]) {
    for(size_t i = 0; i < LAYER_SIZE; i++) {
        for(size_t j = 0; j < LAYER_SIZE; j++) {
            if(layer[i][j] > 0) {
                return true;
            }
        }
    }

    return false;
}

void setup_game(unsigned current_layer[][LAYER_SIZE][LAYER_SIZE], unsigned CARDS, unsigned LAYER_AMOUNT, unsigned card_amounts[]) {
    for(size_t j = 0; j < LAYER_AMOUNT; j++) {
        for(size_t i = 0; i < CARDS; i++) {
            setup_layer(current_layer[j], card_amounts[i], i + 1);
        }
    }
}

void flush_deck(unsigned deck[]) {
    for(size_t i = 0; i < DECK_SIZE; i++) {
        deck[i] = 0;
    }
}
