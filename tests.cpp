
#include "package.hxx"
#include "factory.hxx"
#include "reports.hxx"
#include "storage_types.hxx"
#include "helpers.hxx"
#include "nodes.hxx"
#include <sstream> 
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// ################## TESTY PRODUKTY I ICH PRZECHOWYWANIE
/*
TEST(PackageTest, IsAssignedIdLowest) {
    // przydzielanie ID o jeden większych -- utworzenie dwóch obiektów pod rząd

    Package p1;
    Package p2;

    EXPECT_EQ(p1.get_id(), 1);
    EXPECT_EQ(p2.get_id(), 2);
}

TEST(PackageTest, IsIdReused) {
    // przydzielanie ID po zwolnionym obiekcie

    {
        Package p1;
    }
    Package p2;

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsMoveConstructorCorrect) {
    Package p1;
    Package p2(std::move(p1));

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsAssignmentOperatorCorrect) {
    Package p1;
    Package p2 = std::move(p1);

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageQueueTest, IsFifoCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 1);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 2);
}

TEST(PackageQueueTest, IsLifoCorrect) {
    PackageQueue q(PackageQueueType::LIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 2);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 1);
} */

// ############################################
// ################# TESTY WEZLY
TEST(ReportsTest, GenerateStructureReportMatchesSpecification) {
    Factory factory;
    factory.add_ramp(Ramp(1, 3));
    factory.add_worker(Worker(1, 2, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));
    auto ramp_it = factory.find_ramp_by_id(1);
    auto worker_it = factory.find_worker_by_id(1);
    ramp_it->receiver_preferences_.add_receiver(&(*worker_it));

    std::stringstream ss;
    generate_structure_report(factory, ss);
    
    std::string output = ss.str();
    EXPECT_THAT(output, testing::HasSubstr("LOADING RAMP #1"));
    EXPECT_THAT(output, testing::HasSubstr("  Delivery interval: 3"));
}

TEST(ReportsTest, GenerateSimulationReportHandlesEmptyContainers) {
    Factory factory;
    factory.add_worker(Worker(1, 2, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));
    std::stringstream ss;
    generate_simulation_turn_report(factory, ss, 1);
    std::string output = ss.str();
    EXPECT_THAT(output, testing::HasSubstr("PBuffer: (empty)"));
    EXPECT_THAT(output, testing::HasSubstr("Queue: (empty)"));
    EXPECT_THAT(output, testing::HasSubstr("SBuffer: (empty)"));
}

TEST(ReportsTest, ReceiversAreSortedCorrectlly) {
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    auto ramp_it = factory.find_ramp_by_id(1);
    ramp_it->receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(2)));
    ramp_it->receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(1)));
    ramp_it->receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    std::stringstream ss;
    generate_structure_report(factory, ss);
    std::string output = ss.str();
    size_t posStorehouse = output.find("storehouse #1");
    size_t posWorker1 = output.find("worker #1");
    size_t posWorker2 = output.find("worker #2");
    EXPECT_TRUE(posStorehouse < posWorker1);
    EXPECT_TRUE(posWorker1 < posWorker2);
}


TEST(FactoryLoadingTest, LoadsCorrectFactory) {
    std::string factory_data = R"(
        LOADING_RAMP id=1 delivery-interval=2
        WORKER id=1 processing-time=3 queue-type=FIFO
        STOREHOUSE id=1
        LINK src=ramp-1 dest=worker-1
        LINK src=worker-1 dest=store-1
    )";

    std::istringstream iss(factory_data);

    Factory factory = load_factory_structure(iss);

    EXPECT_TRUE(factory.is_consistent());
}

TEST(FactoryIOTest, ParseLinkInvalidDestThrows) {
// do pracownika którego w ogóle nie ma w systemie.
    std::istringstream iss("LOADING_RAMP id=1 delivery-interval=1\n"
                           "LINK src=ramp-1 dest=worker-99");
    EXPECT_THROW(load_factory_structure(iss), std::runtime_error);
}

TEST(FactoryIOTest, IgnoreCommentsAndEmptyLines)//puste linie ze spacjami 
{
    std::istringstream iss("# To jest komentarz\n\n"
                           "STOREHOUSE id=1\n"
                           "  \n" 
                           "# Kolejny komentarz");
    auto factory = load_factory_structure(iss);

    EXPECT_EQ(factory.storehouse_cbegin()->get_id(), 1);
    EXPECT_EQ(std::distance(factory.storehouse_cbegin(), factory.storehouse_cend()), 1);
}

TEST(FactoryIOTest, SaveComplexLinks)
 {
    //rampa i pracownik zapisują się tak samo
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    // ramoa-worker-magaz
    factory.find_ramp_by_id(1)->receiver_preferences_.add_receiver(factory.find_worker_by_id(1));
    factory.find_worker_by_id(1)->receiver_preferences_.add_receiver(factory.find_storehouse_by_id(1));

    std::ostringstream oss;
    factory.save_factory_structure(oss);
    std::string out = oss.str();
    EXPECT_NE(out.find("LINK src=ramp-1 dest=worker-1"), std::string::npos);
    EXPECT_NE(out.find("LINK src=worker-1 dest=store-1"), std::string::npos);
}


TEST(FactoryIOTest, SaveFlushesStream) 
{ //flush w save_factory_structure
    Factory factory;
    factory.add_storehouse(1);
    
    std::ostringstream oss;
    factory.save_factory_structure(oss);

    EXPECT_TRUE(oss.good());
    EXPECT_FALSE(oss.str().empty());
}
TEST(FactoryTest, IsConsistentCorrect) {
//upel
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    Worker& w = *(factory.find_worker_by_id(1));
    w.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(1)));

    EXPECT_TRUE(factory.is_consistent());
}
TEST(FactoryIOTest, ParseRamp)//upel
 {
    std::istringstream iss("LOADING_RAMP id=1 delivery-interval=3");
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.ramp_cbegin(), 1), factory.ramp_cend());
    const auto& r = *(factory.ramp_cbegin());
    EXPECT_EQ(1, r.get_id());
    EXPECT_EQ(3, r.get_delivery_interval());
}


TEST(WorkerTest, HasBuffer) {
    // Test scenariusza opisanego na stronie:
    // http://home.agh.edu.pl/~mdig/dokuwiki/doku.php?id=teaching:programming:soft-dev:topics:net-simulation:part_nodes#bufor_aktualnie_przetwarzanego_polproduktu

    Worker w(1, 2, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Time t = 1;

    w.receive_package(Package(1));
    w.do_work(t);
    ++t;
    w.receive_package(Package(2));
    w.do_work(t);
    auto& buffer = w.get_sending_buffer();

    ASSERT_TRUE(buffer.has_value());
    EXPECT_EQ(buffer.value().get_id(), 1);
}

TEST(PackageTest, IsAssignedIdLowest) {
    // przydzielanie ID o jeden większych -- utworzenie dwóch obiektów pod rząd

    Package p1;
    Package p2;

    EXPECT_EQ(p1.get_id(), 1);
    EXPECT_EQ(p2.get_id(), 2);
}

TEST(PackageTest, IsIdReused) {
    // przydzielanie ID po zwolnionym obiekcie

    {
        Package p1;
    }
    Package p2;

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsMoveConstructorCorrect) {
    Package p1;
    Package p2(std::move(p1));

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsAssignmentOperatorCorrect) {
    Package p1;
    Package p2 = std::move(p1);

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageQueueTest, IsFifoCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 1);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 2);
}

TEST(PackageQueueTest, IsLifoCorrect) {
    PackageQueue q(PackageQueueType::LIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 2);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 1);
}

// ############################################