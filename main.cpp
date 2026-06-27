#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

namespace Color
{
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string CYAN = "\033[36m";
}

struct Seat
{
    string seatID;
    bool isReserved = false;
    string phoneNumber;
    string destination;
    int fare = 0;
};

struct Destination
{
    string name;
    int fare;
};

class BusReservationSystem
{
public:
    BusReservationSystem()
    {
        initializeSeats();
        loadReservations();
    }

    void run()
    {
        while (true)
        {
            printBoxedTitle("BUS RESERVATION SYSTEM");
            if (hasDestination)
                cout << "Selected destination: " << Color::YELLOW << selectedDestination << Color::RESET << "\n";
            cout << "1. Choose Destination\n"
                 << "2. Reserve a Seat\n"
                 << "3. View Reservations\n"
                 << "4. Cancel a Reservation\n"
                 << "5. Trip Summary\n"
                 << "6. Search by Phone Number\n"
                 << "7. Exit\n";

            int choice = readMenuChoice("Enter your choice: ", 1, 7);

            switch (choice)
            {
            case 1:
                selectedDestination = chooseDestination();
                hasDestination = true;
                cout << Color::GREEN << "Destination set to " << selectedDestination << ".\n"
                     << Color::RESET;
                break;
            case 2:
                if (!hasDestination)
                    cout << Color::RED << "Please choose a destination first (option 1).\n"
                         << Color::RESET;
                else
                    reserveSeat(selectedDestination);
                break;
            case 3:
                viewReservations();
                break;
            case 4:
                cancelReservation();
                break;
            case 5:
                viewSummary();
                break;
            case 6:
                searchByPhone();
                break;
            case 7:
                saveReservations();
                cout << Color::CYAN << "Saving data and exiting. Dhonnobad!\n"
                     << Color::RESET;
                return;
            }
        }
    }

private:
    static const int ROWS = 10;
    static const int COLUMNS = 4;
    const string DATA_FILE = "reservations.dat";

    vector<vector<Seat>> seats;
    vector<Destination> destinations = {
        {"Bhanga", 200},
        {"Lohagara", 300},
        {"Narail", 400},
        {"Jashore", 500},
        {"Benapol", 580}};

    string selectedDestination;
    bool hasDestination = false;

    /* ---------------------- small helpers ---------------------- */

    static void printRule(int len)
    {
        cout << string(len, '-') << "\n";
    }

    static void printBoxedTitle(const string &title)
    {
        int width = (int)title.size() + 4;
        cout << Color::CYAN << Color::BOLD;
        cout << "+" << string(width, '-') << "+\n";
        cout << "|  " << title << "  |\n";
        cout << "+" << string(width, '-') << "+\n";
        cout << Color::RESET;
    }

    /* Reads an integer menu choice, re-prompting until the user
       gives a number inside [min, max]. Never crashes on bad input
       (letters, symbols, empty line, etc.). */
    static int readMenuChoice(const string &prompt, int min, int max)
    {
        int choice;
        while (true)
        {
            cout << prompt;
            if (!(cin >> choice))
            {
                cout << Color::RED << "Invalid input. Please enter a number.\n"
                     << Color::RESET;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (choice < min || choice > max)
            {
                cout << Color::RED << "Please enter a number between " << min << " and " << max << ".\n"
                     << Color::RESET;
                continue;
            }
            return choice;
        }
    }

    static string readToken(const string &prompt)
    {
        string token;
        cout << prompt;
        cin >> token;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return token;
    }

    static bool isAllDigits(const string &s)
    {
        if (s.empty())
            return false;
        return all_of(s.begin(), s.end(), [](unsigned char c)
                      { return isdigit(c) != 0; });
    }

    /* ------------------------- persistence ------------------------- */

    void initializeSeats()
    {
        const string rowLabels = "ABCDEFGHIJ";
        seats.assign(ROWS, vector<Seat>(COLUMNS));
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                seats[i][j].seatID = string(1, rowLabels[i]) + to_string(j + 1);
                seats[i][j].isReserved = false;
                seats[i][j].phoneNumber.clear();
                seats[i][j].destination.clear();
                seats[i][j].fare = 0;
            }
        }
    }

    void saveReservations()
    {
        ofstream fout(DATA_FILE, ios::trunc);
        if (!fout)
        {
            cout << Color::RED << "Warning: could not save reservations to disk.\n"
                 << Color::RESET;
            return;
        }
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                const Seat &s = seats[i][j];
                if (s.isReserved)
                {
                    fout << i << ' ' << j << ' ' << s.phoneNumber << ' ' << s.destination << ' ' << s.fare << '\n';
                }
            }
        }
    }

    void loadReservations()
    {
        ifstream fin(DATA_FILE);
        if (!fin)
            return; /* no saved data yet, that's fine */

        int row, col, fare;
        string phone, dest;
        while (fin >> row >> col >> phone >> dest >> fare)
        {
            if (row >= 0 && row < ROWS && col >= 0 && col < COLUMNS)
            {
                seats[row][col].isReserved = true;
                seats[row][col].phoneNumber = phone;
                seats[row][col].destination = dest;
                seats[row][col].fare = fare;
            }
        }
    }

    /* -------------------------- seat map -------------------------- */

    void displaySeatMap() const
    {
        cout << "\n"
             << Color::BOLD << "Seat Map" << Color::RESET
             << "   (" << Color::GREEN << "open" << Color::RESET << " = available, "
             << Color::RED << "XX" << Color::RESET << " = reserved)\n\n";

        cout << "     ";
        for (int j = 0; j < COLUMNS; j++)
        {
            string num = to_string(j + 1);
            if (num.size() < 2)
                num = " " + num;
            cout << " " << num << " ";
        }
        cout << "\n";

        for (int i = 0; i < ROWS; i++)
        {
            cout << "Row " << string(1, "ABCDEFGHIJ"[i]) << ":";
            for (int j = 0; j < COLUMNS; j++)
            {
                if (!seats[i][j].isReserved)
                    cout << Color::GREEN << " " << string(3 - (int)seats[i][j].seatID.size(), ' ') << seats[i][j].seatID << Color::RESET;
                else
                    cout << Color::RED << "  XX" << Color::RESET;
            }
            cout << "\n";
        }
    }

    /* ----------------------- destinations -------------------------- */

    int getFareByName(const string &name) const
    {
        for (const auto &d : destinations)
            if (d.name == name)
                return d.fare;
        return 0;
    }

    void printDestinationTable() const
    {
        cout << "\n"
             << Color::YELLOW << Color::BOLD
             << setWidth("No.", 4) << " " << setWidth("Destination", 12) << " " << "Fare (Tk)" << Color::RESET << "\n";
        printRule(28);
        for (size_t i = 0; i < destinations.size(); i++)
        {
            cout << setWidth(to_string(i + 1), 4) << " " << setWidth(destinations[i].name, 12) << " " << destinations[i].fare << "\n";
        }
    }

    /* Small left-padding helper so we don't have to pull in <iomanip>
       just for fixed-width text columns. */
    static string setWidth(const string &s, size_t width)
    {
        if (s.size() >= width)
            return s;
        return s + string(width - s.size(), ' ');
    }

    /* Lets the user pick a destination from a numbered list instead
       of typing the name (avoids typos/case mismatches). */
    string chooseDestination() const
    {
        printDestinationTable();
        int choice = readMenuChoice("\nSelect destination number: ", 1, (int)destinations.size());
        return destinations[choice - 1].name;
    }

    /* -------------------------- reserve / cancel -------------------------- */

    void reserveSeat(const string &destination)
    {
        int fare = getFareByName(destination);
        if (fare == 0)
        {
            cout << Color::RED << "Invalid destination.\n"
                 << Color::RESET;
            return;
        }

        displaySeatMap();
        string seatID = readToken("\nEnter Seat ID to Reserve (e.g., A1, B2): ");

        int row = -1, col = -1;
        for (int i = 0; i < ROWS && row == -1; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                if (seats[i][j].seatID == seatID && !seats[i][j].isReserved)
                {
                    row = i;
                    col = j;
                    break;
                }
            }
        }

        if (row == -1)
        {
            cout << Color::RED << "Invalid or already reserved seat.\n"
                 << Color::RESET;
            return;
        }

        string phoneNumber = readToken("Enter Your Phone Number (11 digits): ");
        if (phoneNumber.size() != 11 || !isAllDigits(phoneNumber))
        {
            cout << Color::RED << "Invalid phone number. It must be exactly 11 digits.\n"
                 << Color::RESET;
            return;
        }

        cout << "Destination: " << destination << " | Fare: " << Color::YELLOW << fare << " Tk" << Color::RESET << "\n";
        int paymentConfirmed = readMenuChoice("Confirm Payment? (1 = Yes, 0 = No): ", 0, 1);

        if (paymentConfirmed)
        {
            seats[row][col].isReserved = true;
            seats[row][col].phoneNumber = phoneNumber;
            seats[row][col].destination = destination;
            seats[row][col].fare = fare;
            saveReservations();
            cout << Color::GREEN << "\nSeat " << seatID << " reserved successfully for " << destination
                 << ". Fare paid: " << fare << " Tk.\n"
                 << Color::RESET;
        }
        else
        {
            cout << Color::YELLOW << "Payment not confirmed. Reservation cancelled.\n"
                 << Color::RESET;
        }
    }

    void cancelReservation()
    {
        string seatID = readToken("\nEnter Seat ID to Cancel (e.g., A1, B2): ");

        int row = -1, col = -1;
        for (int i = 0; i < ROWS && row == -1; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                if (seats[i][j].seatID == seatID && seats[i][j].isReserved)
                {
                    row = i;
                    col = j;
                    break;
                }
            }
        }

        if (row == -1)
        {
            cout << Color::RED << "Invalid or non-reserved seat.\n"
                 << Color::RESET;
            return;
        }

        int confirm = readMenuChoice("Are you sure you want to cancel this reservation? (1 = Yes, 0 = No): ", 0, 1);
        if (!confirm)
        {
            cout << "Cancellation aborted.\n";
            return;
        }

        seats[row][col].isReserved = false;
        seats[row][col].phoneNumber.clear();
        seats[row][col].destination.clear();
        seats[row][col].fare = 0;
        saveReservations();
        cout << Color::GREEN << "Reservation for seat " << seatID << " has been cancelled successfully.\n"
             << Color::RESET;
    }

    /* -------------------------- reports -------------------------- */

    void viewReservations() const
    {
        cout << "\n"
             << Color::BOLD << "Current Reservations" << Color::RESET << "\n";
        cout << setWidth("Seat", 6) << " " << setWidth("Destination", 12) << " " << setWidth("Phone", 13) << " " << "Fare (Tk)" << "\n";
        printRule(46);

        bool any = false;
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                const Seat &s = seats[i][j];
                if (s.isReserved)
                {
                    any = true;
                    cout << setWidth(s.seatID, 6) << " " << setWidth(s.destination, 12) << " " << setWidth(s.phoneNumber, 13) << " " << s.fare << "\n";
                }
            }
        }
        if (!any)
            cout << "(no reservations yet)\n";
        cout << "\n";
    }

    void viewSummary() const
    {
        int reserved = 0, revenue = 0;
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLUMNS; j++)
                if (seats[i][j].isReserved)
                {
                    reserved++;
                    revenue += seats[i][j].fare;
                }

        int total = ROWS * COLUMNS;
        int available = total - reserved;

        cout << "\n"
             << Color::BOLD << "Trip Summary" << Color::RESET << "\n";
        printRule(28);
        cout << "Total seats     : " << total << "\n";
        cout << "Reserved seats  : " << reserved << "\n";
        cout << "Available seats : " << available << "\n";
        cout << "Total revenue   : " << Color::YELLOW << revenue << " Tk\n"
             << Color::RESET;
    }

    void searchByPhone() const
    {
        string phone = readToken("\nEnter phone number to search: ");

        bool found = false;
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLUMNS; j++)
            {
                const Seat &s = seats[i][j];
                if (s.isReserved && s.phoneNumber == phone)
                {
                    found = true;
                    cout << Color::GREEN << "Seat " << s.seatID << " -> " << s.destination
                         << ", Fare: " << s.fare << " Tk\n"
                         << Color::RESET;
                }
            }
        }
        if (!found)
            cout << Color::YELLOW << "No reservation found for that phone number.\n"
                 << Color::RESET;
    }
};

int main()
{
    BusReservationSystem system;
    system.run();
    return 0;
}
