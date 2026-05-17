#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <random>
#include <cstdlib>
#include "include/Example.h"


void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

class Card {
private:
    std::string suit;   // "Hearts", "Diamonds", "Clubs", "Spades"
    std::string rank;   // "2".."10", "J", "Q", "K", "A"
    int value;         

    static int computeValue(const std::string& rank) {
        if (rank == "A") return 11;
        if (rank == "K" || rank == "Q" || rank == "J") return 10;
        try {
            return std::stoi(rank);
        } catch (...) {
            throw std::invalid_argument("Invalid card rank: " + rank);
        }
    }

public:
    Card(const std::string& suit, const std::string& rank)
        : suit(suit), rank(rank), value(computeValue(rank)) {
        
        const std::string validSuits[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
        bool validSuit = false;
        for (const auto& s : validSuits) {
            if (s == suit) { validSuit = true; break; }
        }
        
        if (!validSuit)
            throw std::invalid_argument("Invalid suit: " + suit);
    }

    Card(const Card& other)
        : suit(other.suit), rank(other.rank), value(other.value) {}

    Card& operator=(Card other) {
        std::swap(suit, other.suit);
        std::swap(rank, other.rank);
        std::swap(value, other.value);
        return *this;
    }

    ~Card() {}

    const std::string& getSuit() const { return suit; }
    const std::string& getRank() const { return rank; }
    int getValue() const { return value; }

    bool isRed() const {
        return suit == "Hearts" || suit == "Diamonds";
    }

    std::string getColor() const {
        return isRed() ? "Red" : "Black";
    }

    bool isHigherThan(const Card& other) const {
        return value > other.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Card& card) {
        os << card.rank << " of " << card.suit << " [" << card.getColor() << "]";
        return os;
    }
};


class Player {
private:
    std::string name;
    double bankroll;
    double currentBet;
    std::vector<Card> hand;
    int gamesWon;
    int gamesLost;

public:
    Player(const std::string& name, double initialBankroll)
        : name(name), bankroll(initialBankroll), currentBet(0.0), gamesWon(0), gamesLost(0) {
        if (initialBankroll < 0)
            throw std::invalid_argument("Bankroll cannot be negative.");
        if (name.empty())
            throw std::invalid_argument("Player name cannot be empty.");
    }

    ~Player() {}

    void placeBet(double amount) {
        if (amount <= 0)
            throw std::invalid_argument("Bet must be positive.");
        if (amount > bankroll)
            throw std::invalid_argument("Insufficient funds to place this bet.");
        currentBet = amount;
        bankroll -= amount;
    }

    void winBet(double multiplier) {
        bankroll += currentBet * multiplier;
        ++gamesWon;
        currentBet = 0.0;
    }

    void loseBet() {
        ++gamesLost;
        currentBet = 0.0;
    }

    void pushBet() {
        bankroll += currentBet;
        currentBet = 0.0;
    }

    double getBankroll() const { return bankroll; }

    void receiveCard(const Card& card) {
        hand.push_back(card);
    }

    void clearHand() {
        hand.clear();
    }

    const std::vector<Card>& getHand() const { return hand; }
    int handSize() const { return static_cast<int>(hand.size()); }

    const std::string& getName() const { return name; }
    bool isBroke() const { return bankroll < 1.0; }

    void printStats() const {
        std::cout << "=== Stats for " << name << " ===\n"
                  << std::fixed << std::setprecision(2)
                  << "  Bankroll : $" << bankroll << "\n"
                  << "  Won      : " << gamesWon << "\n"
                  << "  Lost     : " << gamesLost << "\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const Player& player) {
        os << player.name << " [Bankroll: $" << std::fixed << std::setprecision(2) << player.bankroll;
        if (player.currentBet > 0)
            os << ", Bet: $" << player.currentBet;
        os << "] Hand: ";
        
        if (player.hand.empty()) {
            os << "(empty)";
        } else {
            for (size_t i = 0; i < player.hand.size(); ++i) {
                os << player.hand[i];
                if (i + 1 < player.hand.size()) os << ", ";
            }
        }
        return os;
    }
};

class Dealer {
private:
    std::vector<Card> hand;
    bool holeCardRevealed;

    int computeHandValue() const {
        int total = 0;
        int aces = 0;
        for (const auto& card : hand) {
            total += card.getValue();
            if (card.getRank() == "A") ++aces;
        }
        while (total > 21 && aces > 0) {
            total -= 10;
            --aces;
        }
        return total;
    }

public:
    Dealer() : holeCardRevealed(false) {}
    ~Dealer() {}

    void receiveCard(const Card& card) {
        hand.push_back(card);
    }

    void clearHand() {
        hand.clear();
        holeCardRevealed = false;
    }

    void revealHoleCard() {
        holeCardRevealed = true;
    }

    int getHandValue() const {
        return computeHandValue();
    }

    bool shouldHit() const {
        return computeHandValue() <= 16;
    }

    bool hasBlackjack() const {
        return hand.size() == 2 && computeHandValue() == 21;
    }

    int handSize() const {
        return static_cast<int>(hand.size());
    }

    const std::vector<Card>& getHand() const {
        return hand;
    }

    friend std::ostream& operator<<(std::ostream& os, const Dealer& dealer) {
        os << "Dealer's hand: ";
        if (dealer.hand.empty()) {
            os << "(empty)";
            return os;
        }

        os << dealer.hand[0];

        for (size_t i = 1; i < dealer.hand.size(); ++i) {
            if (i == 1 && !dealer.holeCardRevealed)
                os << ", [Hidden]";
            else
                os << ", " << dealer.hand[i];
        }

        if (dealer.holeCardRevealed)
            os << " (Total: " << dealer.computeHandValue() << ")";

        return os;
    }
};

class Deck {
private:
    std::vector<Card> cards;
    int dealtCount;

    void populate() {
        const std::string suits[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
        const std::string ranks[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
        cards.clear();
        for (const auto& suit : suits) {
            for (const auto& rank : ranks) {
                cards.emplace_back(suit, rank);
            }
        }
        dealtCount = 0;
    }

public:
    Deck() : dealtCount(0) {
        populate();
    }

    ~Deck() {}

    void shuffle() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(cards.begin(), cards.end(), gen);
        dealtCount = 0;
    }

    Card dealCard() {
        if (isEmpty()) {
            throw std::runtime_error("Deck is empty — cannot deal a card.");
        }
        Card c = cards.back();
        cards.pop_back();
        ++dealtCount;
        return c;
    }

    void reset() {
        populate();
        shuffle();
    }

    int remaining() const {
        return static_cast<int>(cards.size());
    }

    bool isEmpty() const {
        return cards.empty();
    }

    friend std::ostream& operator<<(std::ostream& os, const Deck& deck) {
        os << "Deck [" << deck.remaining() << " cards remaining, " << deck.dealtCount << " dealt]";
        return os;
    }
};

class RidetheBusGame {
private:
    Player& player;
    Deck deck;
    double originalBet;

    static void printCard(const std::string& label, const Card& card) {
        std::cout << "  " << label << ": " << card << "\n";
    }

    static bool stageColorGuess(const Card& card) {
        std::cout << "\n--- STAGE 1: Red or Black? ---\n";
        std::string guess;
        while (true) {
            std::cout << "Your guess (red/black): ";
            std::cin >> guess;
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            if (guess == "red" || guess == "black") break;
            std::cout << "Invalid — enter 'red' or 'black'.\n";
        }
        bool correct = (guess == "red") ? card.isRed() : !card.isRed();
        printCard("Card was", card);
        if (correct) std::cout << "Correct! Card is " << card.getColor() << ". x2 multiplier!\n";
        else std::cout << "Wrong! Card is " << card.getColor() << ". You lose.\n";
        return correct;
    }

    static bool stageHigherLower(const Card& prevCard, const Card& newCard) {
        std::cout << "\n--- STAGE 2: Higher or Lower? ---\n";
        std::cout << "Previous card was: " << prevCard << "\n";
        std::string guess;
        while (true) {
            std::cout << "Higher or lower than " << prevCard.getRank() << "? (higher/lower): ";
            std::cin >> guess;
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            if (guess == "higher" || guess == "lower") break;
            std::cout << "Invalid — enter 'higher' or 'lower'.\n";
        }
        printCard("New card", newCard);
        if (newCard.getValue() == prevCard.getValue()) {
            std::cout << "Same value! You lose.\n";
            return false;
        }
        bool correct = (guess == "higher") ? newCard.isHigherThan(prevCard) : prevCard.isHigherThan(newCard);
        if (correct) std::cout << "Correct! x3 multiplier!\n";
        else std::cout << "Wrong! You lose.\n";
        return correct;
    }

    static bool stageInsideOutside(const Card& low, const Card& high, const Card& newCard) {
        std::cout << "\n--- STAGE 3: Inside or Outside? ---\n";
        int lo = std::min(low.getValue(), high.getValue());
        int hi = std::max(low.getValue(), high.getValue());
        std::cout << "  Range to beat: " << lo << " to " << hi << "\n";
        
        std::string guess;
        while (true) {
            std::cout << "Inside or outside the range [" << lo << "-" << hi << "]? (inside/outside): ";
            std::cin >> guess;
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            if (guess == "inside" || guess == "outside") break;
            std::cout << "Invalid — enter 'inside' or 'outside'.\n";
        }

        printCard("New card", newCard);
        if (newCard.getValue() == lo || newCard.getValue() == hi) {
            std::cout << "Card landed exactly on the boundary! You lose everything.\n";
            return false;
        }

        bool isInside = (newCard.getValue() > lo && newCard.getValue() < hi);
        bool correct = (guess == "inside") ? isInside : !isInside;
        if (correct) std::cout << "Correct! x4 multiplier!\n";
        else std::cout << "Wrong! You lose.\n";
        return correct;
    }

    static bool stageSuitGuess(const Card& card) {
        std::cout << "\n--- STAGE 4: Guess the Suit! ---\n";
        std::cout << "Options: hearts / diamonds / clubs / spades\n";
        std::string guess;
        while (true) {
            std::cout << "Your guess: ";
            std::cin >> guess;
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            if (guess == "hearts" || guess == "diamonds" || guess == "clubs" || guess == "spades") break;
            std::cout << "Invalid — enter hearts, diamonds, clubs, or spades.\n";
        }
        guess[0] = static_cast<char>(std::toupper(guess[0]));
        printCard("Card drawn", card);
        bool correct = (guess == card.getSuit());
        if (correct) std::cout << "INCREDIBLE! Correct suit! x10 multiplier!\n";
        else std::cout << "Wrong! Suit was " << card.getSuit() << ". You lose.\n";
        return correct;
    }

    int cashout(double multiplier) const {
        std::string choice;
        while (true) {
            std::cout << "Amount won so far: $" << originalBet * multiplier << "\n";
            std::cout << "Do you want to cash out your winnings? (y/n): ";
            std::cin >> choice;
            std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
            if (choice == "y" || choice == "Y") {
                double winnings = originalBet * multiplier;
                player.winBet(multiplier);
                std::cout << "Cashing out... You win $" << winnings << "!\n";
                return 1;
            } else if (choice == "n" || choice == "N") {
                return 0;
            } else {
                std::cout << "Invalid — enter 'y' or 'n'.\n";
            }
        }
    }

public:
    explicit RidetheBusGame(Player& p) : player(p), originalBet(0.0) {
        deck.shuffle();
    }

    ~RidetheBusGame() {}

    void playRound() {
        std::cout << "\n====== RIDE THE BUS ======\n";
        while (!player.isBroke()) {
            std::cout << "\n-------------------------------------\n";
            std::cout << std::fixed << std::setprecision(2) << "Current Bankroll: $" << player.getBankroll() << "\n";

            if (deck.remaining() < 10) deck.reset();

            double bet = -1;
            while (true) {
                std::cout << "Enter your bet (or 0 to leave table): $";
                std::cin >> bet;
                if (std::cin.fail() || bet < 0 || bet > player.getBankroll()) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid bet. You have $" << player.getBankroll() << "\n";
                    bet = -1;
                } else {
                    break;
                }
            }
            
            if (bet == 0) {
                std::cout << "Leaving Ride the Bus table...\n";
                break;
            }
            clearScreen();

            player.placeBet(bet);
            originalBet = bet;

            Card c1 = deck.dealCard();
            if (!stageColorGuess(c1)) { player.loseBet(); continue; }

            if(cashout(2.0) == 1) continue;

            Card c2 = deck.dealCard();
            if (!stageHigherLower(c1, c2)) { player.loseBet(); continue; }

            if(cashout(3.0) == 1) continue;

            Card c3 = deck.dealCard();
            if (!stageInsideOutside(c1, c2, c3)) { player.loseBet(); continue; }

            if(cashout(4.0) == 1) continue;

            Card c4 = deck.dealCard();
            if (!stageSuitGuess(c4)) { player.loseBet(); continue; }

            double winnings = originalBet * 10.0;
            player.winBet(10.0);
            std::cout << "\n AMAZING! You rode the bus all the way! Won $" << winnings << "!\n";
        }
        
        if (player.isBroke()) {
            std::cout << "You're out of money! Returning to main menu...\n";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const RidetheBusGame& game) {
        os << "RidetheBusGame | " << game.deck << " | Player: " << game.player.getName();
        return os;
    }
};

class BlackjackGame {
    Player& player;
    Dealer dealer;
    Deck deck;

    static int computePlayerHandValue(const Player& p) {
        int total = 0, aces = 0;
        for (const auto& card : p.getHand()) {
            total += card.getValue();
            if (card.getRank() == "A") ++aces;
        }
        while (total > 21 && aces > 0) { total -= 10; --aces; }
        return total;
    }

    void dealInitialCards() {
        player.clearHand();
        dealer.clearHand();
        if (deck.remaining() < 15) deck.reset();
        player.receiveCard(deck.dealCard());
        dealer.receiveCard(deck.dealCard());
        player.receiveCard(deck.dealCard());
        dealer.receiveCard(deck.dealCard());
    }

        void printTable(bool fullReveal) const {
            std::cout << "\n--- TABLE ---\n";
            if (fullReveal) std::cout << dealer << "\n";
            else {
                const auto& dHand = dealer.getHand();
                std::cout << "Dealer's hand: " << dHand[0] << ", [Hidden]\n";
            }
            std::cout << player << "\n";
            std::cout << "  Your hand value: " << computePlayerHandValue(player) << "\n";
            if(!fullReveal) std::cout << "  Dealer's visible card value: " << dealer.getHand()[0].getValue() << "\n";
            std::cout << "-------------\n";
        }

    void playerTurn() {
        while (true) {
            printTable(false);
            int val = computePlayerHandValue(player);
            if (val >= 21) {
                clearScreen();
                if (val == 21) std::cout << "You have 21!\n";
                else std::cout << "BUST! Your hand value: " << val << "\n";
                break;
            }
            std::cout << "Choose action: (h)it / (s)tand: ";
            std::string choice;
            std::cin >> choice;
            if (choice == "h" || choice == "hit") player.receiveCard(deck.dealCard());
            else if (choice == "s" || choice == "stand") {clearScreen(); break;}
        }
    }

    void dealerTurn() {
        dealer.revealHoleCard();
        //clearScreen();
        std::cout << "\nDealer reveals: "<< "\n" << dealer << "\n";
        while (dealer.shouldHit()) {
            Card c = deck.dealCard();
            std::cout << "Dealer hits: " << c << "\n"; 
            dealer.receiveCard(c);
        }
        std::cout << "Dealer stands. Final hand: " << "\n" << dealer << "\n";
    }

    void resolveRound() {
        //clearScreen();
        int pVal = computePlayerHandValue(player);
        int dVal = dealer.getHandValue();
        std::cout << "\n=== RESULT ===\nYour total: " << pVal << " | Dealer total: " << dVal << "\n";

        bool pBJ = (player.handSize() == 2 && pVal == 21);
        bool dBJ = dealer.hasBlackjack();

        if (pVal > 21) { std::cout << "You BUSTED.\n"; player.loseBet(); }
        else if (dVal > 21) { std::cout << "Dealer BUSTED. You WIN!\n"; player.winBet(2.0); }
        else if (pBJ && !dBJ) { std::cout << "BLACKJACK! You win 3:2!\n"; player.winBet(2.5); }
        else if (dBJ && !pBJ) { std::cout << "Dealer has Blackjack. You lose.\n"; player.loseBet(); }
        else if (pVal > dVal) { std::cout << "You WIN!\n"; player.winBet(2.0); }
        else if (pVal < dVal) { std::cout << "You lose.\n"; player.loseBet(); }
        else { std::cout << "PUSH — tie.\n"; player.pushBet(); }
    }

public:
    explicit BlackjackGame(Player& p) : player(p) { deck.shuffle(); }
    void playRound() {
        std::cout << "\n====== WELCOME TO BLACKJACK ======\n";
        
        while (!player.isBroke()) {
            std::cout << "\n-------------------------------------\n";
            std::cout << std::fixed << std::setprecision(2) << "Current Bankroll: $" << player.getBankroll() << "\n";
            
            double bet = -1;
            while (true) {
                std::cout << "Enter your bet (or 0 to leave table): $";
                if (!(std::cin >> bet) || bet < 0 || bet > player.getBankroll()) {
                    std::cin.clear(); std::cin.ignore(1000, '\n');
                    std::cout << "Invalid bet.\n"; bet = -1;
                } else {
                    break;
                }
            }
            
            if (bet == 0) {
                std::cout << "Leaving Blackjack table...\n";
                break;
            }

            player.placeBet(bet);
            dealInitialCards();
            playerTurn();
            if (computePlayerHandValue(player) <= 21) dealerTurn();
            resolveRound();
        }
        
        if (player.isBroke()) {
            std::cout << "You're out of money! Returning to main menu...\n";
        }
    }
friend std::ostream& operator<<(std::ostream& os, const BlackjackGame& game) {
    os << "BlackjackGame | " << game.deck << " | Player: " << game.player.getName();
    return os;
}
};

static void printMenu() {
    std::cout << "      CARD GAME HUB\n";
    std::cout << "------------------------------\n";
    std::cout << "  1. Blackjack\n";
    std::cout << "  2. Ride the Bus\n";
    std::cout << "  3. View Stats\n";
    std::cout << "  0. Exit\n";
    std::cout << "------------------------------\n";
    std::cout << "Choice: ";
}

static int readInt() {
    int val;
    std::cin >> val;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    return val;
}

int main() {

    Example ex;
    ex.g();

    std::cout << "Welcome to the Card Game Hub!\n";
    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) name = "Player";

    double startBankroll = 0;
    while (startBankroll <= 0) {
        std::cout << "Enter starting bankroll ($): ";
        std::cin >> startBankroll;
        if (std::cin.fail() || startBankroll <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter a positive amount.\n";
            startBankroll = 0;
        }
    }

    try {
        Player player(name, startBankroll);
        BlackjackGame bjGame(player);
        RidetheBusGame rtbGame(player);

        std::cout << "\n" << bjGame << "\n";
        std::cout << rtbGame << "\n";

        bool running = true;
        while (running) {
            if (player.isBroke()) {
                std::cout << "\nYou're broke! Game over.\n";
                break;
            }

            printMenu();
            int choice = readInt();

            switch (choice) {
                case 1:
                    try {
                        bjGame.playRound();
                    } catch (const std::exception& e) {
                        std::cout << "Error in Blackjack: " << e.what() << "\n";
                    }
                    break;
                case 2:
                    try {
                        rtbGame.playRound();
                    } catch (const std::exception& e) {
                        std::cout << "Error in Ride the Bus: " << e.what() << "\n";
                    }
                    break;
                case 3:
                    player.printStats();
                    break;
                case 0:
                    running = false;
                    std::cout << "Thanks for playing!\n";
                    break;
                default:
                    std::cout << "Invalid choice.\n";
            }
        }

        std::cout << "\n    FINAL STATS \n";
        player.printStats();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
