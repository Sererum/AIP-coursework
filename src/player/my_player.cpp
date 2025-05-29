#include "my_player.hpp"
#include <cstring>
#include <cstdlib>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

Point MyPlayer::make_move(const State &state) {
  Point result;
  if (state.get_move_no() == 0) {
    result.x = state.get_opts().cols / 2;
    result.y = state.get_opts().rows / 2;
    return result;
  }
  for (int n_attempt = 0; n_attempt < 10; ++n_attempt) {
    result.x = std::rand() % state.get_opts().cols;
    result.y = std::rand() % state.get_opts().rows;
    if (state.get_value(result.x, result.y) != Sign::NONE) {
      --n_attempt;
      continue;
    }
    bool has_neighbors = false;
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        if (dx == 0 && dy == 0)
          continue;
        const Sign val = state.get_value(result.x + dx, result.y + dy);
        if (val != Sign::NONE) {
          has_neighbors = true;
          break;
        }
      }
      if (has_neighbors)
        break;
    }
    if (has_neighbors)
      break;
  }
  return result;
}

const int ROMBO[5][5] = {
    {0, 0, 9, 0, 0},
    {0, 9, 1, 9, 0},
    {9, 1, 0, 1, 9},
    {0, 9, 1, 9, 0},
    {0, 0, 9, 0, 0}
};

void MyBot::init_fields(int size_x, int size_y) {
    this->size_x = size_x;
    this->size_y = size_y;

    own_field = new bool*[size_y];
    enemy_field = new bool*[size_y];

    for (int y = 0; y < size_y; ++y) {
        own_field[y] = new bool[size_x];
        enemy_field[y] = new bool[size_x];
        std::memset(own_field[y], 0, size_x * sizeof(bool));
        std::memset(enemy_field[y], 0, size_x * sizeof(bool));
    }
}

void MyBot::free_fields() {
    for (int y = 0; y < size_y; ++y) {
        delete[] own_field[y];
        delete[] enemy_field[y];
    }
    delete[] own_field;
    delete[] enemy_field;
}

bool MyBot::isWin(bool** field, int size) {
    const int WIN_LEN = 5;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!field[y][x]) continue;

            // Горизонталь
            if (x + WIN_LEN <= size) {
                int count = 0;
                for (int i = 0; i < WIN_LEN; ++i)
                    if (field[y][x + i]) ++count;
                if (count == WIN_LEN) return true;
            }

            // Вертикаль
            if (y + WIN_LEN <= size) {
                int count = 0;
                for (int i = 0; i < WIN_LEN; ++i)
                    if (field[y + i][x]) ++count;
                if (count == WIN_LEN) return true;
            }

            // Диагональ ↘
            if (x + WIN_LEN <= size && y + WIN_LEN <= size) {
                int count = 0;
                for (int i = 0; i < WIN_LEN; ++i)
                    if (field[y + i][x + i]) ++count;
                if (count == WIN_LEN) return true;
            }

            // Диагональ ↗
            if (x + WIN_LEN <= size && y - WIN_LEN >= -1) {
                int count = 0;
                for (int i = 0; i < WIN_LEN; ++i)
                    if (field[y - i][x + i]) ++count;
                if (count == WIN_LEN) return true;
            }
        }
    }
    return false;
}

Point MyBot::winPose(bool** field1, bool** field2, int size) {
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!field1[y][x] && !field2[y][x]) {
                field1[y][x] = true;
                if (isWin(field1, size)) {
                    field1[y][x] = false;
                    return {x, y};
                }
                field1[y][x] = false;
            }
        }
    }
    return {-1, -1};
}

void MyBot::get_weights(bool** field, int** weights, int size, double rate) {
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (field[y][x]) {
                for (int dy = -2; dy <= 2; ++dy) {
                    for (int dx = -2; dx <= 2; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < size && ny >= 0 && ny < size) {
                            weights[ny][nx] += ROMBO[dy + 2][dx + 2] * rate;
                        }
                    }
                }
            }
        }
    }
}

Point MyBot::get_best_move(int** weights, bool** own, bool** enemy, int size) {
    Point best = {-1, -1};
    int max_weight = -1;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (!own[y][x] && !enemy[y][x]) {
                if (weights[y][x] > max_weight) {
                    max_weight = weights[y][x];
                    best = {x, y};
                }
            }
        }
    }

    return best;
}

void MyBot::update_field(Sign sign, bool** field, const State& state) {
    for (int y = 0; y < size_y; ++y) {
        for (int x = 0; x < size_x; ++x) {
            bool has = (state.get_value(x, y) == sign);
            if (has != field[y][x]) {
                field[y][x] = has;
            }
        }
    }
}

void MyBot::set_sign(Sign sign) {
    m_sign = sign;
}

const char* MyBot::get_name() const {
    return m_name;
}

Point MyBot::make_move(const State& state) {
    if (size_x == 0 || size_y == 0) {
        size_x = state.get_opts().cols;
        size_y = state.get_opts().rows;
        init_fields(size_x, size_y);
    }

    update_field(m_sign, own_field, state);
    Sign enemy_sign = (m_sign == Sign::X) ? Sign::O : Sign::X;
    update_field(enemy_sign, enemy_field, state);

    // Проверка: есть ли победный ход?
    Point win = winPose(own_field, enemy_field, size_y);
    if (win.x != -1) return win;

    // Нужно ли блокировать победу противника?
    Point block = winPose(enemy_field, own_field, size_y);
    if (block.x != -1) return block;

    // Считаем веса
    int** weights = new int*[size_y];
    for (int y = 0; y < size_y; ++y) {
        weights[y] = new int[size_x]{0};
    }

    get_weights(own_field, weights, size_y, 1.0);     // свои ходы
    get_weights(enemy_field, weights, size_y, 0.8);   // чужие ходы

    // Выбираем лучший ход
    Point result = get_best_move(weights, own_field, enemy_field, size_y);

    for (int y = 0; y < size_y; ++y) {
        delete[] weights[y];
    }
    delete[] weights;

    if (result.x == -1) {
        result.x = size_x / 2;
        result.y = size_y / 2;
    }

    return result;
}

}; // namespace ttt::my_player
