#pragma once

#include "core/game.hpp"

namespace ttt::my_player {

using game::Event;
using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

class MyPlayer : public IPlayer {
  Sign m_sign = Sign::NONE;
  const char *m_name;

public:
  MyPlayer(const char *name) : m_sign(Sign::NONE), m_name(name) {}
  void set_sign(Sign sign) override;
  Point make_move(const State &game) override;
  const char *get_name() const override;
};

class MyBot : public game::IPlayer {
public:
    using game::IPlayer::IPlayer;

	MyBot(const char* name): m_name(name) {}

    void set_sign(Sign sign) override;
    Point make_move(const State &state) override;
    const char *get_name() const override;

private:
    Sign m_sign = Sign::NONE;
    const char* m_name;

    int size_x = 0;
    int size_y = 0;

    bool** own_field = nullptr;
    bool** enemy_field = nullptr;

    void init_fields(int size_x, int size_y);
    void free_fields();

    bool isWin(bool** field, int size);
    Point winPose(bool** field1, bool** field2, int size);

    static void get_weights(bool** field, int** weights, int size, double rate);
    Point get_best_move(int** weights, bool** own, bool** enemy, int size);
    void update_field(Sign sign, bool** field, const State& state);
};

}; // namespace ttt::my_player
