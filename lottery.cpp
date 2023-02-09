#include <algorithm>
#include <ctime>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <unordered_set>

template <typename T, typename Container>
class Interlayer : public Container {
public:
  T& operator[](size_t index) {
    return this->c[index];
  }

  const T& operator[](size_t index) const {
    return this->c[index];
  }

  template <typename Comparator>
  void sort(Comparator func) {
    std::sort(this->c.begin(), this->c.end(), func);
  }
};

template <template <typename...> typename T>
class Game;

size_t show_progress(size_t current, size_t total, const std::string& caption, size_t prev_progress_value, bool erase = false);

template <typename T, typename Container, typename Q = int>
std::string shrink_list_view(Interlayer<T, Container>& list, size_t length_to_end, bool lead_zero = true, size_t count_items_near_shrinking = 4, size_t max_count_without_shrinking = 9);

std::mt19937 MT(time(nullptr));

size_t rnd_gen() {
  return MT();
}

template <typename T, typename Container = T>
void shuffle(Container& list, size_t count, bool progress_show = false) {
  if (!count)
    return;

  for (size_t i = 0, progress; i < count; ++i) {
    std::swap(list[i], list[rnd_gen() % (i + 1)]);

    if (progress_show)
      progress = show_progress(i, count, std::string("Shuffling"), progress, true);
  }
}

class Ticket {
public:
  static const size_t price = 100;
  static const size_t rows = 6;
  static const size_t cols = 5;
  static const unsigned char max_num = 90;

  const size_t id;

  Ticket(size_t id) : id(id) {
    generate_nums();
  }

  const unsigned char num(size_t index) {
    return nums_[index];
  }

  void set_purchased(bool purchased) {
    purchased_ = purchased;
  }

  bool is_purchased() const {
    return purchased_;
  }

  void set_winner(bool winner) {
    winner_ = winner;
  }

  bool is_winner() const {
    return winner_;
  }

  void set_prize(size_t prize) {
    if (winner_)
      return;

    prize_ = prize;
  }

  size_t prize() const {
    return prize_;
  }

private:
  unsigned char nums_[rows * cols];
  bool purchased_ = false;
  bool winner_ = false;
  size_t prize_ = 0;

  void generate_nums() {
    bool bitmap[max_num];

    for (size_t i = 0; i < max_num; ++i)
      bitmap[i] = false;

    for (size_t i = 0, value; i < rows * cols; ++i) {
      value = rnd_gen() % max_num;

      while (bitmap[value])
        value = (value + 1) % max_num;

      bitmap[value] = true;
      nums_[i] = static_cast<unsigned char>(value + 1);
    }
  }
};

template <template <typename...> typename T>
class Round {
public:
  const bool missed_numbers;
  const Interlayer<unsigned char, T<unsigned char>> combination;
  const Interlayer<Ticket*, T<Ticket*>> winners;
  const size_t prize;

  Round(Interlayer<unsigned char, T<unsigned char>> combination, Interlayer<Ticket*, T<Ticket*>>& winners, size_t prize, bool missed_numbers = false) : missed_numbers(missed_numbers), combination(combination), winners(winners), prize(prize) {}

  ~Round() {}
};

template <template <typename...> typename T>
class Edition {
public:
  const size_t id;
  const size_t min_id;
  const size_t count;
  const size_t jackpot_fund;

  static const size_t kJackpotCountSteps = 15;

  Edition(size_t id, size_t count, size_t min_id, size_t jackpot_fund) : id(id), min_id(min_id), count(count), jackpot_fund(jackpot_fund) {
    std::string caption = "Generating ";
    caption += std::to_string(count);
    caption += " tickets";

    for (size_t i = 0, progress = 0; i < count; ++i) {
      Ticket* ticket = new Ticket(min_id + i);
      tickets_.push(ticket);

      progress = show_progress(i, count, caption, progress);
    }
  }

  ~Edition() {
    std::string caption = "Deleting rounds of edition ";
    caption += std::to_string(id);

    if (jackpot_)
      delete jackpot_;

    for (size_t i = 0, progress = 0; i < rounds_.size(); ++i) {
      delete rounds_[i];

      progress = show_progress(i, rounds_.size(), caption, progress);
    }

    caption = "Deleting edition ";
    caption += std::to_string(id);

    for (size_t i = 0, progress = 0; i < count; ++i) {
      delete tickets_[i];

      progress = show_progress(i, count, caption, progress);
    }
  }

  bool sell(size_t sell_count) {
    if (sold_ || !active_)
      return false;

    sell_count_ = sell_count;

    Interlayer<size_t, T<size_t>> random_list;

    for (size_t i = 0; i < count; ++i)
      random_list.push(i);

    shuffle<Interlayer<size_t, T<size_t>>>(random_list, count, true);

    std::string caption = "Selling ";
    caption += std::to_string(sell_count_);
    caption += " tickets";

    for (size_t i = 0, progress = 0; i < sell_count_; ++i) {
      tickets_[random_list[i]]->set_purchased(true);

      progress = show_progress(i, sell_count_, caption, progress);
    }

    sold_ = true;

    return true;
  }

  bool draw(Interlayer<unsigned char, T<unsigned char>>& combination, size_t round_number, size_t count_equal_nums, size_t total_count_balls, size_t& count_round_combination, size_t adj_show_nums, size_t& prize_fund, bool& ruined_fund) {
    if (!active_)
      return false;

    Interlayer<unsigned char, T<unsigned char>> round_combination;
    count_round_combination = combination.size() - adj_show_nums;

    for (size_t i = 0; i < count_round_combination; ++i)
      round_combination.push(combination[combination.size() - count_round_combination + i]);

    std::string caption = "Searching for balls ";
    caption += shrink_list_view<unsigned char, T<unsigned char>>(round_combination, count_round_combination);

    std::unordered_set<unsigned char> combination_set;
    combination_set.reserve(combination.size());

    for (size_t i = 0; i < combination.size(); ++i)
      combination_set.insert(combination[i]);

    Interlayer<Ticket*, T<Ticket*>> winners;

    for (size_t i = 0, progress = 0; i < count; ++i) {
      progress = show_progress(i, count, caption, progress, true);

      if (!tickets_[i]->is_purchased() || tickets_[i]->is_winner())
        continue;

      bool winner = false;

      for (size_t j = 0; j < Ticket::rows * Ticket::cols; ++j) {
        if (combination.back() == tickets_[i]->num(j)) {
          size_t begin = j / count_equal_nums * count_equal_nums;
          size_t k = 0;

          while (true) {
            if (!combination_set.count(tickets_[i]->num(begin + k)))
              break;
            else if (++k == count_equal_nums) {
              winner = true;
              break;
            }
          }
        }

        if (winner)
          break;
      }

      if (winner)
        winners.push(tickets_[i]);
    }

    if (winners.size() || total_count_balls + 1 == Ticket::max_num) {
      size_t prize_round;

      if (total_count_balls != kJackpotCountSteps - 1 || round_number != 1)
        prize_round = allocation_fund(round_number, winners.size(), prize_fund, ruined_fund);
      else
        prize_round = jackpot_fund / winners.size();

      for (size_t i = 0; i < winners.size(); ++i) {
        winners[i]->set_prize(prize_round);
        winners[i]->set_winner(true);
      }

      Round<T>* round = new Round<T>(round_combination, winners, prize_round);

      if (total_count_balls != kJackpotCountSteps - 1 || round_number != 1)
        rounds_.push(round);
      else
        jackpot_ = round;

      count_winners_ += winners.size();

      return true;
    }

    return false;
  }

  bool set_missed_numbers(Interlayer<unsigned char, T<unsigned char>>& combination, size_t adj_show_nums) {
    if (set_missed_already_ || !active_)
      return false;

    Interlayer<unsigned char, T<unsigned char>> missed_combination;

    for (size_t i = 0; i < combination.size() - adj_show_nums; ++i)
      missed_combination.push(combination[adj_show_nums + i]);

    Interlayer<Ticket*, T<Ticket*>> empty;
    Round<T>* missed = new Round<T>(missed_combination, empty, 0, true);
    rounds_.push(missed);

    set_missed_already_ = true;

    return true;
  }

  Ticket* ticket(size_t pos) {
    return tickets_[pos];
  }

  Round<T>* round(size_t pos) {
    return rounds_[pos];
  }

  size_t round_count() {
    return rounds_.size();
  }

  Round<T>* jackpot() {
    return jackpot_;
  }

  bool is_active() const {
    return active_;
  }

  void disable() {
    active_ = false;
  }

  bool is_sold() const {
    return sold_;
  }

  size_t fund() const {
    return fund_;
  }

  bool set_fund(size_t fund) {
    if (!active_ || !sold_)
      return false;

    fund_ = fund;

    return true;
  }

  size_t sell_count() const {
    return sell_count_;
  }

  size_t count_winners() const {
    return count_winners_;
  }

private:
  Interlayer<Ticket*, T<Ticket*>> tickets_;
  Interlayer<Round<T>*, T<Round<T>*>> rounds_;
  Round<T>* jackpot_ = nullptr;

  bool active_ = true;
  bool sold_ = false;
  bool set_missed_already_ = false;

  size_t fund_ = 0;
  size_t sell_count_ = 0;
  size_t count_winners_ = 0;

  size_t allocation_fund(size_t round_number, size_t count_winners, size_t& prize_fund, bool& ruined_fund) const {
    ++round_number;

    size_t prize = 0;
    size_t total_prize = 0;

    if (round_number == 1) {
      total_prize = 500000;
    } else if (round_number == 2) {
      prize = 5000000;
    } else if (round_number >= 3 && round_number <= 6) {
      prize = 1000000;
    } else if (round_number == 7) {
      total_prize = 500000;
    } else if (round_number >= 8 && round_number <= 12) {
      prize = 10000;
    } else if (round_number >= 13 && round_number <= 15) {
      prize = 5000;
    } else if (round_number >= 16 && round_number <= 18) {
      prize = 1000;
    } else if (round_number >= 19 && round_number <= 21) {
      prize = 500;
    } else if (round_number >= 22 && round_number <= 24) {
      prize = 300;
    } else if (round_number >= 25 && round_number <= 27) {
      prize = 200;
    } else {
      prize = 100;
    }

    if (prize_fund / count_winners < prize || total_prize && prize_fund < total_prize) {
      prize = prize_fund / count_winners;
      total_prize = prize * count_winners;

      ruined_fund = true;
    }

    if (prize && !total_prize)
      total_prize = prize * count_winners;
    else if (total_prize && !prize)
      prize = total_prize / count_winners;

    if (prize_fund <= total_prize)
      prize_fund = 0;
    else
      prize_fund -= total_prize;

    return prize;
  }
};

template <template <typename...> typename T>
class Game {
public:
  const double kPercentagePrizeFund = 0.5;

  Game() {}

  ~Game() {
    if (!last_edit_)
      return;

    for (size_t i = 0; i <= last_edit_id_; ++i)
      delete editions_[i];
  }

  void add() {
    if (last_edit_)
      last_edit_->disable();

    size_t count = sub_cmd<size_t>("Number of tickets in new edition");

    if (!count) {
      std::cout << "Number of tickets can only be positive" << std::endl;
      return;
    }

    jackpot_fund_ += sub_cmd<size_t>("Add to jackpot fund");

    if (sub_cmd<bool>("Add last fund balance to jackpot fund", false, true)) {
      jackpot_fund_ += last_fund_balance_;
      last_fund_balance_ = 0;
    }

    simulate_jackpot_ = sub_cmd<bool>("Simulate jackpot", true, true);

    Edition<T>* edition = new Edition<T>(++last_edit_id_, count, count_, jackpot_fund_);
    editions_.push(edition);

    last_edit_ = editions_[last_edit_id_];
    count_ += count;

    std::cout << "Jackpot fund: " << last_edit_->jackpot_fund << std::endl;
  }

  void sell() {
    if (!last_edit_) {
      std::cout << "Last edition does not exist" << std::endl;
      return;
    }

    if (last_edit_->is_sold()) {
      std::cout << "Last edition already sold" << std::endl;
      return;
    }

    double percentage = sub_cmd<double>("Percentage of tickets will be sold", true);

    if (percentage <= 0.0 || percentage > 100.0) {
      std::cout << "Percentage can only be in the range (0, 100]" << std::endl;
      return;
    }

    size_t sell_count = percentage * last_edit_->count / 100;

    if (!sell_count)
      sell_count = 1;

    if (last_edit_->sell(sell_count)) {
      size_t fund = kPercentagePrizeFund * Ticket::price * sell_count;

      if (last_edit_->set_fund(fund))
        std::cout << "Prize fund: " << fund << std::endl;
      else
        std::cout << "Fund setting error" << std::endl;
    }
  }

  void play() {
    if (!last_edit_) {
      std::cout << "Last edition does not exist" << std::endl;
      return;
    }

    if (!last_edit_->is_active()) {
      std::cout << "Last edition was disabled" << std::endl;
      return;
    }

    if (!last_edit_->is_sold()) {
      std::cout << "Last edition was not sold" << std::endl;
      return;
    }

    unsigned char balls[Ticket::max_num];

    for (size_t i = 0; i < Ticket::max_num; ++i)
      balls[i] = i + 1;

    shuffle<unsigned char>(balls, Ticket::max_num);

    if (simulate_jackpot_) {
      size_t rnd_ticket = rnd_gen() % last_edit_->count;

      while (!last_edit_->ticket(rnd_ticket)->is_purchased())
        rnd_ticket = (rnd_ticket + 1) % last_edit_->count;

      for (size_t i = 0; i < Edition<T>::kJackpotCountSteps; ++i) {
        for (size_t j = i + 1; j < Ticket::max_num; ++j) {
          if (balls[j] == last_edit_->ticket(rnd_ticket)->num(i))
            std::swap(balls[i], balls[j]);
        }
      }

      shuffle<unsigned char>(balls, Edition<T>::kJackpotCountSteps);
    }

    Interlayer<unsigned char, T<unsigned char>> combination;

    bool jackpot_shown = false;

    std::cout << "Round 1" << std::endl;

    size_t round_number = 0;
    size_t prev_round = 0;
    size_t count_equal_nums;
    size_t count_round_combination = 0;
    size_t adj_show_nums = 0;

    last_fund_balance_ = last_edit_->fund();

    bool ruined_fund = false;

    for (size_t i = 0; i < Ticket::max_num; ++i) {
      combination.push(balls[i]);

      if (ruined_fund || i + 1 == Ticket::max_num) {
        if (i + 1 < Ticket::max_num)
          continue;

        std::cout << "Missed numbers" << std::endl;
        last_edit_->set_missed_numbers(combination, adj_show_nums);
        show_round(last_edit_->round(round_number));
        std::cout << std::endl;
        break;
      }

      if (round_number == 0)
        count_equal_nums = Ticket::cols;
      else
        count_equal_nums = Ticket::rows * Ticket::cols / (round_number == 1 ? 2 : 1);

      if (round_number != prev_round) {
        std::cout << "Round " << (round_number + 1) << std::endl;
        prev_round = round_number;
      }

      if (combination.size() < count_equal_nums)
        continue;

      if (last_edit_->draw(combination, round_number, count_equal_nums, i, count_round_combination, adj_show_nums, last_fund_balance_, ruined_fund)) {
        Round<T>* rounds_;

        if (!last_edit_->jackpot() || jackpot_shown) {
          round = last_edit_->round(round_number);

          adj_show_nums += count_round_combination;
          count_round_combination = 0;
          ++round_number;
        } else {
          jackpot_fund_ = 0;

          round = last_edit_->jackpot();
          jackpot_shown = true;

          std::cout << "Jackpot!" << std::endl;
        }

        show_round(round);

        std::cout << std::endl;
      }
    }

    std::cout << "Game over!" << std::endl;
    std::cout << "  Participated tickets: " << last_edit_->sell_count() << std::endl;
    std::cout << "  Total winners: " << last_edit_->count_winners() << std::endl;
    std::cout << "  Fund balance: " << last_fund_balance_ << std::endl;

    last_edit_->disable();
  }

  void show() {
    switch (sub_cmd<size_t>("Ticket[1], edition[2] or any to exit")) {
    case 1: {
      show_ticket(sub_cmd<size_t>("Ticket ID", true));
      break;
    }
    case 2: {
      show_edition(sub_cmd<size_t>("Edition ID", true));
      break;
    }
    }
  }

  void show_round(Round<T>* round) const {
    const size_t kMaxCountShowingIds = 10;

    std::cout << "  Combination: ";

    if (!round->combination.size())
      std::cout << "(empty)";

    for (size_t i = 0; i < round->combination.size(); ++i)
      std::cout << (i ? ", " : "") << (round->combination[i] < 10 ? "0" : "") << static_cast<int>(round->combination[i]);

    std::cout << std::endl;

    if (round->missed_numbers)
      return;

    std::cout << "  " << round->winners.size() << " winners: ";

    if (!round->winners.size())
      std::cout << "(empty)";

    for (size_t i = 0; i < round->winners.size(); ++i) {
      std::cout << (i ? ", " : "") << round->winners[i]->id;

      if (i == kMaxCountShowingIds - 1) {
        std::cout << ", ...";
        break;
      }
    }

    std::cout << std::endl;

    std::cout << "  Prize: " << round->prize << std::endl;
  }

  void show_ticket(size_t id) const {
    if (!last_edit_) {
      std::cout << "No tickets at all" << std::endl;
      return;
    }

    Ticket* ticket = nullptr;
    size_t edit_id = 0;

    for (; edit_id <= last_edit_id_; ++edit_id) {
      if (id >= editions_[edit_id]->min_id && id < editions_[edit_id]->min_id + editions_[edit_id]->count) {
        ticket = editions_[edit_id]->ticket(id - editions_[edit_id]->min_id);
        break;
      }
    }

    if (!ticket) {
      std::cout << "Ticket not found" << std::endl;
      return;
    }

    for (size_t i = 0; i < Ticket::rows * Ticket::cols; ++i) {
      std::cout << (ticket->num(i) < 10 ? "0" : "") << static_cast<int>(ticket->num(i));

      if ((i + 1) % Ticket::cols == 0) {
        std::cout << "  :  ";

        if (i + 1 == Ticket::cols * 1)
          std::cout << "ID: " << ticket->id;
        else if (i + 1 == Ticket::cols * 2)
          std::cout << "Edition: " << edit_id << " (" << (editions_[edit_id]->is_active() ? "active, " : "not active, ") << (editions_[edit_id]->is_sold() ? "sold" : "not sold") << ")";
        else if (i + 1 == Ticket::cols * 3)
          std::cout << "Purchased: " << (ticket->is_purchased() ? "yes" : "no");
        else if (i + 1 == Ticket::cols * 4)
          std::cout << "Winner: " << (ticket->is_winner() ? "yes" : "no");
        else if (i + 1 == Ticket::cols * 5)
          std::cout << "Prize: " << ticket->prize();

        std::cout << std::endl;
      } else
        std::cout << " | ";
    }
  }

  void show_edition(size_t id) const {
    if (!last_edit_) {
      std::cout << "No editions at all" << std::endl;
      return;
    }

    if (id > last_edit_id_) {
      std::cout << "Edition not found" << std::endl;
      return;
    }

    Edition<T>* edit = editions_[id];

    std::cout << "ID: " << id << " (" << (edit->is_active() ? "active, " : "not active, ") << (edit->is_sold() ? "sold" : "not sold") << ")" << std::endl;
    std::cout << "Ticket IDs: " << edit->min_id << " to " << (edit->min_id + edit->count - 1) << std::endl;
    std::cout << "Number of tickets: " << edit->count << std::endl;
    std::cout << "Participated tickets: " << edit->sell_count() << std::endl;
    std::cout << "Total winners: " << edit->count_winners() << std::endl;
    std::cout << "Prize fund: " << edit->fund() << std::endl;
    std::cout << "Jackpot fund: " << edit->jackpot_fund << " (" << (edit->jackpot() ? "spent" : "unspent") << ")" << std::endl;

    if (edit->jackpot()) {
      std::cout << std::endl;
      std::cout << "Jackpot" << std::endl;
      show_round(edit->jackpot());
    }

    for (size_t i = 0; i < edit->round_count(); ++i) {
      std::cout << std::endl;

      if (!edit->round(i)->missed_numbers)
        std::cout << "Round " << (i + 1) << std::endl;
      else
        std::cout << "Missed numbers" << std::endl;

      show_round(edit->round(i));
    }
  }

  void search() const {
    if (!last_edit_) {
      std::cout << "No tickets at all" << std::endl;
      return;
    }

    size_t type_editions = sub_cmd<size_t>("Search in each[1] or specific[2] edition or any to exit");
    size_t edit_id = 0;
    size_t end_id_edit = 0;

    if (type_editions == 1)
      end_id_edit = last_edit_id_ + 1;
    else if (type_editions == 2) {
      while ((edit_id = sub_cmd<size_t>("Edition ID")) > last_edit_id_)
        std::cout << "Edition not found" << std::endl;

      end_id_edit = edit_id + 1;
    }

    if (!end_id_edit)
      return;

    size_t type_search = sub_cmd<size_t>("Prizes[1], jackpots[2] or any to exit");
    Interlayer<Ticket*, T<Ticket*>> list;

    if (type_search == 1) {
      size_t min = sub_cmd<size_t>("Min prize");
      size_t max;

      while ((max = sub_cmd<size_t>("Max prize")) < min)
        std::cout << "Max prize >= min prize" << std::endl;

      for (size_t i = edit_id; i < end_id_edit; ++i) {
        for (size_t j = 0; j < editions_[i]->round_count(); ++j) {
          if (editions_[i]->round(j)->prize < min || editions_[i]->round(j)->prize > max)
            continue;

          for (size_t k = 0; k < editions_[i]->round(j)->winners.size(); ++k)
            list.push(editions_[i]->round(j)->winners[k]);
        }
      }
    } else if (type_search == 2) {
      for (size_t i = edit_id; i < end_id_edit; ++i) {
        if (!editions_[i]->jackpot())
          continue;

        for (size_t j = 0; j < editions_[i]->jackpot()->winners.size(); ++j)
          list.push(editions_[i]->jackpot()->winners[j]);
      }
    } else
      return;

    if (!list.size()) {
      std::cout << std::endl;
      std::cout << "Empty result" << std::endl;
      return;
    } else if (list.size() > 1) {
      switch (sub_cmd<size_t>("Sort results by ID[1], by prize[2] or any without sorting")) {
      case 1:
        list.sort([](Ticket* a, Ticket* b) { return a->id < b->id; });
        break;
      case 2:
        list.sort([](Ticket* a, Ticket* b) { return (a->prize() > b->prize()) || (a->prize() == b->prize() && a->id < b->id); });
        break;
      }
    }

    std::cout << std::endl;
    std::cout << "Found " << list.size() << " tickets" << std::endl;
    std::cout << std::endl;
    size_t start = sub_cmd<size_t>("Show results from number");
    size_t count = sub_cmd<size_t>("Show as many results (0 to exit)");

    while (true) {
      if (!count)
        return;

      if (start > list.size() - 1)
        start = list.size() - 1;

      if (start + count > list.size())
        count = list.size() - start;

      std::cout << std::endl;
      std::cout << count << " results starting from number " << start << std::endl;
      std::cout << std::endl;

      for (size_t i = start; i < start + count; ++i) {
        show_ticket(list[i]->id);
        std::cout << std::endl;
      }

      start = sub_cmd<size_t>("Show results from number");
      count = sub_cmd<size_t>("Show as many results (0 to exit)");
    }
  }

  void help() const {
    std::cout << "Available commands: add, sell, play, show, search, help, exit" << std::endl;
  }

private:
  Interlayer<Edition<T>*, T<Edition<T>*>> editions_;
  Edition<T>* last_edit_ = nullptr;
  size_t count_ = 0;
  size_t last_edit_id_ = -1;
  size_t last_fund_balance_ = 0;
  size_t jackpot_fund_ = 0;
  bool simulate_jackpot_ = false;

  template <typename Type>
  Type sub_cmd(const char* caption, bool separate_lines = false, bool boolean = false) const {
    Type value;
    std::string ch;

    while (true) {
      if (!boolean) {
        std::cout << ">> " << caption << ": ";
        std::cin >> value;
      } else {
        std::cout << ">> " << caption << "(y/n): ";
        std::cin >> ch;
      }

      if (!std::cin.fail()) {
        if (boolean)
          value = ch == "y";

        break;
      } else {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
      }
    }

    if (separate_lines)
      std::cout << std::endl;

    return value;
  }
};

void splash();

int main(int argc, char** argv) {
  splash();

  Game<std::queue> game;

  std::string cmd;

  while (true) {
    std::cout << std::endl;
    std::cout << "> ";
    std::cin >> cmd;
    std::cout << std::endl;

    if (cmd == "add")
      game.add();
    else if (cmd == "sell")
      game.sell();
    else if (cmd == "play")
      game.play();
    else if (cmd == "show")
      game.show();
    else if (cmd == "search")
      game.search();
    else if (cmd == "help")
      game.help();
    else if (cmd == "exit")
      break;
  }

  return 0;
}

size_t show_progress(size_t current, size_t total, const std::string& caption, size_t prev_progress_value, bool erase) {
  const size_t kProgressBarWidth = 40;

  size_t progress = (current + 1) * 100 / total;

  if (progress == prev_progress_value && current)
    return prev_progress_value;

  prev_progress_value = progress;

  std::cout.flush();

  size_t pos = progress * kProgressBarWidth / 100;

  if (progress < 100)
    std::cout << caption << " [" << std::string(pos, '=') << std::string(kProgressBarWidth - pos, ' ') << "] " << progress << "% " << '\r';
  else {
    std::cout << std::string(kProgressBarWidth + 16 + caption.length(), ' ') << "\r";

    if (!erase)
      std::cout << caption << " [" << std::string(kProgressBarWidth, '=') << "] 100% " << std::endl;
  }

  return prev_progress_value;
}

template <typename T, typename Container, typename Q = int>
std::string shrink_list_view(Interlayer<T, Container>& list, size_t length_to_end, bool lead_zero, size_t count_items_near_shrinking, size_t max_count_without_shrinking) {
  std::string result;

  bool shrink_list = false;

  for (size_t i = 0; i < length_to_end; ++i) {
    if (shrink_list) {
      if (i < length_to_end - count_items_near_shrinking)
        continue;
      else
        shrink_list = false;
    }

    if (i)
      result += ", ";

    if (length_to_end > max_count_without_shrinking &&
        i == count_items_near_shrinking) {
      shrink_list = true;
      result += "...";
      continue;
    }

    if (lead_zero && list[list.size() - length_to_end + i] < 10)
      result += "0";

    result += std::to_string(static_cast<Q>(list[list.size() - length_to_end + i]));
  }

  return result;
}

void splash() {
  std::cout << " _______________________________________________________ " << std::endl;
  std::cout << "|   _          _   _                                    |" << std::endl;
  std::cout << "|  | |    ___ | |_| |_ ___ _ __ _   _                   |" << std::endl;
  std::cout << "|  | |   / _ \\| __| __/ _ \\ '__| | | |                  |" << std::endl;
  std::cout << "|  | |__| (_) | |_| ||  __/ |  | |_| |                  |" << std::endl;
  std::cout << "|  |_____\\___/ \\__|\\__\\___|_|   \\__, |                  |" << std::endl;
  std::cout << "|                               |___/                   |" << std::endl;
  std::cout << "|   ____  _                 _       _   _               |" << std::endl;
  std::cout << "|  / ___|(_)_ __ ___  _   _| | __ _| |_(_) ___  _ __    |" << std::endl;
  std::cout << "|  \\___ \\| | '_ ` _ \\| | | | |/ _` | __| |/ _ \\| '_ \\   |" << std::endl;
  std::cout << "|   ___) | | | | | | | |_| | | (_| | |_| | (_) | | | |  |" << std::endl;
  std::cout << "|  |____/|_|_| |_| |_|\\__,_|_|\\__,_|\\__|_|\\___/|_| |_|  |" << std::endl;
  std::cout << "|                                                       |" << std::endl;
  std::cout << "|                           Created by Daniel Shevtsov  |" << std::endl;
  std::cout << "|_______________________Compiled: " << __DATE__ << " " << __TIME__ << "__|" << std::endl;
}