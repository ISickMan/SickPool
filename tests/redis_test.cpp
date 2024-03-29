// #include <fmt/format.h>
// #include <gtest/gtest.h>
// #include <hiredis/hiredis.h>

// #include "block_submission.hpp"
// #include "redis/redis_manager.hpp"
// #include "redis_round.hpp"
// #include "test_utils.hpp"
// class RedisTest : public ::testing::Test
// {
//    protected:
//     void SetUp() override
//     {
//         using namespace std::string_view_literals;
//         redisCommand(redis_manager.rc, "FLUSHDB");
//         // auto miner = "RSicKPooLFbBeWZEgVrAkCxfAkPRQYwSnC"sv;
//         // auto worker = "worker"sv;
//         // memcpy(submission.miner, miner.data(), ADDRESS_LEN);
//         // memcpy(submission.worker, worker.data(), worker.size());
//         // memcpy(submission.chain, chain, strlen(chain));
//         // test database index
//     }

//     // void TearDown() override { redisCommand(redis_manager.rc, "FLUSHDB"); }

//     const char* chain = "GTEST";
//     CoinConfig c = {.symbol = "GTEST", .redis = {.redis_port = 6379}};
//     RedisManager redis_manager{"127.0.0.1", &c, 1};

//     // BlockSubmission submission{.confirmations = rand(),
//     //                            .block_reward = rand(),
//     //                            .time_ms = rand(),
//     //                            .duration_ms = rand(),
//     //                            .height = (uint32_t)rand(),
//     //                            .number = (uint32_t)rand(),
//     //                            .difficulty = (double)rand(),
//     //                            .effort_percent = (double)rand()};
//     // void TearDown() override {}
// };

// // for testing!
// bool operator==(const Share& a, const Share& b)
// {
//     return memcmp(&a, &b, sizeof(Share)) == 0;
// }

// // template <typename T1, typename T2>
// // bool CompareSpans(const T1& a, const T2& b)
// // {
// //     return a.size() == b.size() &&
// //      !memcmp(a.data(), b.data(), a.size());
// // }

// // TEST_F(RedisTest, GetLastNSharesExact)
// // {
// //     RedisRound rr(redis_manager);

// //     // addresses are in binary
// //     const double progress = 3.0;
// //     const std::vector<Share> test_shares = {Share{{std::byte{1}}, 0.1},
// //                                             Share{{std::byte{2}}, 1.0},
// //                                             Share{{std::byte{3}}, progress}};

// //     const std::vector<Share> exp = {
// //         Share{{std::byte{3}}, progress},
// //     };
// //     const size_t n = 2;
// //     rr.AddPendingShares(test_shares);

// //     const auto [shares, res] = rr.GetLastNShares(progress, n);

// //     CompareSpans(shares, exp);

// //     // check that it removes the unnecessary shares
// //     const auto [shares1, res1] = rr.GetSharesBetween(0, -1);
// //     CompareSpans(shares1, exp);
// // }

// // TEST_F(RedisTest, GetLastNSharesOdd)
// // {
// //     RedisRound rr(redis_manager);

// //     // addresses are in binary
// //     double progress = 3.0;
// //     const std::vector<Share> test_shares = {Share{{std::byte{1}}, 0.9},
// //                                             Share{{std::byte{2}}, 1.5},
// //                                             Share{{std::byte{3}}, progress}};

// //     const std::vector<Share> exp = {Share{{std::byte{2}}, 1.5},
// //                                     Share{{std::byte{3}}, progress}};
// //     const size_t n = 2;
// //     rr.AddPendingShares(test_shares);

// //     // expect the last share to be the remaining
// //     const auto& [shares, _] = rr.GetLastNShares(progress, n);

// //     CompareSpans(shares, exp);

// //     // check that it removes the unnecessary shares
// //     const auto [shares1, res1] = rr.GetSharesBetween(0, -1);
// //     CompareSpans(shares1, exp);
// // }

// // // TEST_F(RedisTest, AddBlockSubmission)
// // // {
// // //     using namespace std::string_view_literals;

// // //     std::string block_key = fmt::format("block:{}", submission.number);

// // //     auto miner = "RSicKPooLFbBeWZEgVrAkCxfAkPRQYwSnC"sv;
// // //     auto worker = "worker"sv;
// // //     memcpy(submission.miner, miner.data(), ADDRESS_LEN);
// // //     memcpy(submission.worker, worker.data(), worker.size());
// // //     memcpy(submission.chain, chain, strlen(chain));

// // //     bool res = redis_manager.AppendAddBlockSubmission(&submission);

// // //     ASSERT_TRUE(res);

// // //     // make sure everything is in the database
// // //     auto reply = (redisReply*)redisCommand(rc, "GET %s",
// // block_key.c_str());

// // //     BlockSubmission received = *((BlockSubmission*)reply->str);

// // //     ASSERT_EQ(memcmp(&submission, &received, sizeof(BlockSubmission)), 0);

// // //     // check one index exists, assume the rest do too as they use same
// // func
// // //     reply = (redisReply*)redisCommand(rc, "ZSCORE block-index:number %u",
// // //                                       submission.number);
// // //     ASSERT_STREQ(reply->str, std::to_string(submission.number).c_str());
// // // }

// // // TEST_F(RedisTest, AddBlockSubmission)
// // // {
// // //     using namespace std::string_view_literals;

// // //     std::string block_key = fmt::format("block:{}", submission.number);

// // //     auto miner = "RSicKPooLFbBeWZEgVrAkCxfAkPRQYwSnC"sv;
// // //     auto worker = "worker"sv;
// // //     memcpy(submission.miner, miner.data(), ADDRESS_LEN);
// // //     memcpy(submission.worker, worker.data(), worker.size());
// // //     memcpy(submission.chain, chain, strlen(chain));

// // //     bool res = redis_manager.AppendAddBlockSubmission(&submission);

// // //     ASSERT_TRUE(res);

// // //     // make sure everything is in the database
// // //     auto reply = (redisReply*)redisCommand(rc, "GET %s",
// // block_key.c_str());

// // //     BlockSubmission received = *((BlockSubmission*)reply->str);

// // //     ASSERT_EQ(memcmp(&submission, &received, sizeof(BlockSubmission)), 0);

// // //     // check one index exists, assume the rest do too as they use same
// // func
// // //     reply = (redisReply*)redisCommand(rc, "ZSCORE block-index:number %u",
// // //                                       submission.number);
// // //     ASSERT_STREQ(reply->str, std::to_string(submission.number).c_str());
// // // }

// // // TEST_F(RedisTest, UpdateBlockConfirmation)
// // // {
// // //     const int confirmations = 10;

// // //     bool res = redis_manager.AppendAddBlockSubmission(&submission);

// // // redis_manager.UpdateBlockConfirmations(std::to_string(submission.number),
// // //                                            confirmations);
// // //     auto reply =
// // //         (redisReply*)redisCommand(rc, "GET block:%u", submission.number);
// // //     BlockSubmission received = *((BlockSubmission*)reply->str);

// // //     ASSERT_EQ(confirmations, received.confirmations);
// // // }

// // // TEST_F(RedisTest, SetMinerShares)
// // // {
// // //     const char* addr = "ADDRESS";
// // //     std::vector<std::pair<std::string, RoundShare>> miner_shares{
// // //         {addr, RoundShare{.effort = 1.f, .share = 0.1, .reward = 1}}};

// // //     bool res = redis_manager.AppendAddRoundShares(chain, &submission,
// // miner_shares);

// // //     ASSERT_TRUE(res);

// // //     // make sure our share was saved (immature reward)
// // //     auto reply = (redisReply*)redisCommand(rc, "HGET immature-rewards:%u
// // %s",
// // //                                            submission.number, addr);

// // //     RoundShare* received = ((RoundShare*)reply->str);

// // //     ASSERT_EQ(memcmp(&miner_shares[0].second, received,
// // sizeof(RoundShare)), 0);
// // // }

// // // TEST_F(RedisTest, UpdateImmatureRewardsConfirmed)
// // // {
// // //     std::string addr = "GTEST_ADDR_CONFIRMED";
// // //     std::vector<std::pair<std::string, RoundShare>> miner_shares{
// // //         {addr, RoundShare{.effort = 1.f, .share = 0.1, .reward = 1}}};

// // //     bool res = redis_manager.AppendAddRoundShares(chain, &submission,
// // miner_shares);

// // //     // matured
// // //     res =
// // //         redis_manager.UpdateImmatureRewards(chain, submission.number, 0,
// // true);

// // //     ASSERT_TRUE(res);

// // //     redisReply* reply = (redisReply*)redisCommand(
// // //         rc, "HGET %s:balance:immature %s", chain, addr.c_str());

// // //     // no immature balance
// // //     ASSERT_STREQ(reply->str, "0");

// // //     reply = (redisReply*)redisCommand(rc, "HGET %s:balance:mature %s",
// // chain,
// // //                                       addr.c_str());

// // //     // exactly same mature balance
// // //     ASSERT_STREQ(reply->str,
// // //                  std::to_string(miner_shares[0].second.reward).c_str());

// // //     // clean up
// // //     redisCommand(rc, "DEL %s:balance:mature %s", chain, addr.c_str());
// // // }

// // // TEST_F(RedisTest, UpdateImmatureRewardsOrphaned)
// // // {
// // //     std::string addr = "GTEST_ADDR_ORPHAN";
// // //     std::vector<std::pair<std::string, RoundShare>> miner_shares{
// // //         {addr, RoundShare{.effort = 1.f, .share = 0.1, .reward = 1}}};

// // //     bool res = redis_manager.AppendAddRoundShares(chain, &submission,
// // miner_shares);

// // //     // orphaned
// // //     res =
// // //         redis_manager.UpdateImmatureRewards(chain, submission.number, 0,
// // false);

// // //     ASSERT_TRUE(res);

// // //     redisReply* reply = (redisReply*)redisCommand(
// // //         rc, "HGET %s:balance:immature %s", chain, addr.c_str());

// // //     // no immature balance
// // //     ASSERT_STREQ(reply->str, "0");

// // //     reply = (redisReply*)redisCommand(rc, "HGET %s:balance:mature %s",
// // chain,
// // //                                       addr.c_str());

// // //     // no mature balance
// // //     ASSERT_STREQ(reply->str, "0");

// // //     // clean up
// // //     redisCommand(rc, "DEL %s:balance:mature %s", chain, addr.c_str());
// // // }

// // TEST_F(RedisTest, AddStakingPoints)
// // {
// //     bool res = redis_manager.AddStakingPoints(chain, 100);

// //     ASSERT_TRUE(res);
// // }

// // // TEST_F(RedisTest, GetPosPoints)
// // // {
// // //     std::vector<std::pair<std::string, double>> stakers_effort;
// // //     bool res = redis_manager.GetPosPoints(stakers_effort, chain);

// // //     ASSERT_TRUE(res);
// // // }

// // // TEST_F(RedisTest, TsCreate)
// // // {
// // //     using namespace std::string_view_literals;

// // //     redis_manager.TsCreate("TS_CREATE_TEST"sv, 1,
// // //                            {{"TEST_LABEL"sv, "TEST_LABEL_VAL"sv}});

// // //     auto reply = (redisReply*)redisCommand(
// // //         rc, "TS.QUERYINDEX TEST_LABEL=TEST_LABEL_VAL");
// // //     ASSERT_EQ(reply->elements, 1);

// // //     // clean up
// // //     redisCommand(rc, "DEL TS_CREATE_TEST");
// // // }

// // // TEST_F(RedisTest, UpdateStats)
// // // {
// // //     worker_map worker_stats_map = {
// // //         {"GTEST_ADDR.worker", WorkerStats{10, 20, 5, 10, 20}}};

// // //     miner_map miner_stats_map = {
// // //         {"GTEST_ADDR", MinerStats{10, 20, 5, 10, 20}}};

// // //     std::mutex mutex;

// // //     bool res = redis_manager.UpdateEffortStats(miner_stats_map, 100,
// // &mutex);

// // //     ASSERT_TRUE(res);
// // // }

// // // TEST_F(RedisTest, GetHashrateAt)
// // // {
// // //     using namespace std::string_literals;
// // //     std::vector<std::pair<std::string, double>> hashrates;
// // //     bool res = redis_manager.GetHashratesAt(hashrates, "miner", 0);

// // //     ASSERT_TRUE(res);

// // //     ASSERT_EQ(hashrates[0].first, "GTEST_ADDR");
// // //     ASSERT_EQ(hashrates[0].second, 20);
// // // }