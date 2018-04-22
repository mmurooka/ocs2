/*
 * testLogicRulesMachine.cpp
 *
 *  Created on: Dec 19, 2017
 *      Author: farbod
 */


#include <ocs2_core/logic/machine/LogicRulesMachine.h>

#include <gtest/gtest.h>

using namespace ocs2;

template <size_t STATE_DIM, size_t INPUT_DIM>
class TestLogicRules : public LogicRulesBase<STATE_DIM,INPUT_DIM>
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef LogicRulesBase<STATE_DIM,INPUT_DIM> BASE;
	typedef typename BASE::scalar_t scalar_t;
	typedef typename BASE::scalar_array_t scalar_array_t;
	typedef typename BASE::size_array_t size_array_t;
	typedef typename BASE::controller_t controller_t;

	TestLogicRules() {}

	virtual ~TestLogicRules() {}

	void adjustController(controller_t& controller) const override
	{}

	void set(const scalar_array_t& eventTimes) {
		BASE::eventTimes_ = eventTimes;
	}

	void update() override
	{}

	void rewind(const scalar_t& lowerBoundTime,
			const scalar_t& upperBoundTime) override
	{}


private:

};

/**
 * Check solution
 * @param logicRulesMachine
 * @param eventTimesStockResult
 * @param switchedSystemIDsStockResult
 * @return
 */
template <size_t STATE_DIM, size_t INPUT_DIM>
bool checkSolution(
		const LogicRulesMachine<STATE_DIM,INPUT_DIM,TestLogicRules<STATE_DIM,INPUT_DIM>>& logicRulesMachine,
		const std::vector<std::vector<double>>& eventTimesStockResult,
		const std::vector<std::vector<size_t>>& switchedSystemIDsStockResult) {

	size_t numPartitionings = eventTimesStockResult.size();

	bool testPass = true;
	for (size_t i=0; i<numPartitionings; i++) {
		if (eventTimesStockResult[i]!=logicRulesMachine.getEventTimes(i))
			if (eventTimesStockResult[i].empty() && logicRulesMachine.getSwitchingTimes(i).empty() )
				continue;
			else {
				testPass = false;
			}
	}
	for (size_t i=0; i<numPartitionings; i++) {
		if (switchedSystemIDsStockResult[i] != logicRulesMachine.getEventCounters(i))
			testPass = false;
	}

	return testPass;
}


TEST(testLogicRulesMachine, LogicRulesMachine)
{
	TestLogicRules<1,1> logicRules;
	LogicRulesMachine<1,1,TestLogicRules<1,1>> logicRulesMachine(logicRules);

	LogicRulesMachine<1,1,TestLogicRules<1,1>>::controller_array_t controllerStock;

	std::vector<double> partitioningTimes{0,1,2,3};
	controllerStock.resize(partitioningTimes.size()-1);

	std::vector<double> logicRulesEventTimes;

	std::cout << std::endl;

	// result
	bool testPass;
	std::vector<std::vector<double>> eventTimesStockResult(3);
	std::vector<std::vector<size_t>> switchedSystemIDsStockResult(3);

	// No switch
	logicRulesEventTimes = std::vector<double>{};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### No switch:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{0};

	eventTimesStockResult[1] = std::vector<double>{};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{0};

	eventTimesStockResult[2] = std::vector<double>{};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{0};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);

	// switches at the end of partitions
	logicRulesEventTimes = std::vector<double>{0, 1, 2, 3};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Switches at the end of partitions:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{1};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{1};

	eventTimesStockResult[1] = std::vector<double>{2};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{2};

	eventTimesStockResult[2] = std::vector<double>{3};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{3};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);

	// swiches after time interval
	logicRulesEventTimes = std::vector<double>{3, 4, 5, 6};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Switches after time interval:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{0};

	eventTimesStockResult[1] = std::vector<double>{};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{0};

	eventTimesStockResult[2] = std::vector<double>{3};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{0};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);

	// switches before time interval
	logicRulesEventTimes = std::vector<double>{-3, -2, -1, 0};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Switches before time interval:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{4};

	eventTimesStockResult[1] = std::vector<double>{};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{4};

	eventTimesStockResult[2] = std::vector<double>{};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{4};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);

	// switches in the middle
	logicRulesEventTimes = std::vector<double>{0, 0.5, 1.5, 2.5};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Switches in the middle:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{0.5};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{1,2};

	eventTimesStockResult[1] = std::vector<double>{1.5};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{2,3};

	eventTimesStockResult[2] = std::vector<double>{2.5};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{3,4};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);

	// no switch in middle partition
	logicRulesEventTimes = std::vector<double>{0.5, 2.5};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### No switch in middle partition:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult[0] = std::vector<double>{0.5};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{0,1};

	eventTimesStockResult[1] = std::vector<double>{};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{1};

	eventTimesStockResult[2] = std::vector<double>{2.5};
	switchedSystemIDsStockResult[2] = std::vector<size_t>{1,2};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);
}

TEST(testLogicRulesMachine, shortPartition)
{
	TestLogicRules<1,1> logicRules;
	LogicRulesMachine<1,1,TestLogicRules<1,1>> logicRulesMachine(logicRules);

	std::vector<double> partitioningTimes;
	LogicRulesMachine<1,1,TestLogicRules<1,1>>::controller_array_t controllerStock;

	std::vector<double> logicRulesEventTimes{0.5, 2.5};
	logicRules.set(logicRulesEventTimes);
	logicRulesMachine.setLogicRules(logicRules);

	std::cout << std::endl;

	// result
	bool testPass;
	std::vector<std::vector<double>> eventTimesStockResult;
	std::vector<std::vector<size_t>> switchedSystemIDsStockResult;

	// Small time partition with one event
	partitioningTimes = std::vector<double>{1.5, 2.0, 3.0};
	controllerStock.resize(partitioningTimes.size()-1);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Small time partition with one event:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult.resize(2);
	switchedSystemIDsStockResult.resize(2);

	eventTimesStockResult[0] = std::vector<double>{};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{1};

	eventTimesStockResult[1] = std::vector<double>{2.5};
	switchedSystemIDsStockResult[1] = std::vector<size_t>{1, 2};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);


	// Small time partition with no event
	partitioningTimes = std::vector<double>{0.5, 1.0, 2.0, 2.5};
	controllerStock.resize(partitioningTimes.size()-1);
	logicRulesMachine.updateLogicRules(partitioningTimes, controllerStock);

	std::cout << std::endl << "======================" << std::endl;
	std::cout << "### Small time partition with no event:" << std::endl;
	logicRulesMachine.display();

	eventTimesStockResult.resize(1);
	switchedSystemIDsStockResult.resize(1);

	eventTimesStockResult[0] = std::vector<double>{};
	switchedSystemIDsStockResult[0] = std::vector<size_t>{1};

	testPass = checkSolution(logicRulesMachine, eventTimesStockResult, switchedSystemIDsStockResult);
	ASSERT_TRUE(testPass);
}

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


