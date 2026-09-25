#include "multiply.hpp"

#include "pause.hpp"

#include <pthread.h>

#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

struct WorkState {
	const Matrix* a = nullptr;
	const Matrix* b = nullptr;
	Matrix* c = nullptr;
	int nextIndex = 0;
	int finished = 0;
	int delayMs = 0;
	bool log = false;
	pthread_mutex_t mutex = {};
	pthread_cond_t done = {};

	WorkState() = default;
	WorkState(const WorkState&) = delete;
	WorkState& operator=(const WorkState&) = delete;
};

// свой экземпляр на поток: номер разный, состояние общее
// объект живёт, пока multiplyParallel не сделает pthread_join
struct WorkerStart {
	WorkState* state;
	int id;
};

// false: свободных клеток нет, главному потоку уже отправлено сообщение
// замок держится только на время «взять номер», пауза и расчёт снаружи
bool claimCell(WorkState& state, const int workerId, int& row, int& col)
{
	pthread_mutex_lock(&state.mutex);

	const int cellCount = state.a->rows() * state.b->cols();
	if (state.nextIndex >= cellCount) {
		++state.finished;
		pthread_cond_signal(&state.done);
		if (state.log) {
			std::cout << "Поток " << workerId << " отправил сообщение: работа закончена\n";
			std::cout.flush();
		}
		pthread_mutex_unlock(&state.mutex);
		return false;
	}

	const int index = state.nextIndex;
	++state.nextIndex;

	// строка = индекс / k, столбец = индекс % k
	const int k = state.b->cols();
	row = index / k;
	col = index % k;

	if (state.log) {
		std::cout << "Поток " << workerId
			<< " взял элемент c[" << (row + 1) << "][" << (col + 1) << "]\n";
		std::cout.flush();
	}

	pthread_mutex_unlock(&state.mutex);
	return true;
}

void* workerRoutine(void* raw)
{
	WorkerStart* start = static_cast<WorkerStart*>(raw);
	WorkState* state = start->state;
	const int workerId = start->id;

	while (true) {
		int row = 0;
		int col = 0;
		if (!claimCell(*state, workerId, row, col)) {
			break;
		}

		pauseMilliseconds(state->delayMs);

		const long long value = state->a->productCell(*state->b, row, col);
		state->c->at(row, col) = value;

		if (state->log) {
			pthread_mutex_lock(&state->mutex);
			std::cout << "Поток " << workerId
				<< " посчитал c[" << (row + 1) << "][" << (col + 1)
				<< "] = " << value << '\n';
			std::cout.flush();
			pthread_mutex_unlock(&state->mutex);
		}
	}

	return nullptr;
}

void joinWorkers(std::vector<pthread_t>& threads, const int created)
{
	for (int i = 0; i < created; ++i) {
		pthread_join(threads[i], nullptr);
	}
}

}  // namespace

Matrix multiplyParallel(
	const Matrix& a,
	const Matrix& b,
	const int threadCount,
	const int delayMs,
	const bool log)
{
	if (threadCount <= 0) {
		throw std::invalid_argument("Число потоков должно быть больше нуля");
	}
	if (a.cols() != b.rows()) {
		throw std::invalid_argument("Для умножения столбцы левой матрицы должны совпасть со строками правой");
	}

	Matrix c(a.rows(), b.cols());
	const int cellCount = a.rows() * b.cols();
	if (log && threadCount > cellCount) {
		std::cout << "Потоков больше, чем клеток C. Лишние сразу сообщат, что работы нет.\n";
	}

	WorkState state;
	state.a = &a;
	state.b = &b;
	state.c = &c;
	state.delayMs = delayMs;
	state.log = log;

	pthread_mutex_init(&state.mutex, nullptr);
	pthread_cond_init(&state.done, nullptr);

	std::vector<pthread_t> threads(static_cast<std::size_t>(threadCount));
	std::vector<WorkerStart> starts(static_cast<std::size_t>(threadCount));
	int created = 0;
	for (int i = 0; i < threadCount; ++i) {
		starts[static_cast<std::size_t>(i)].state = &state;
		starts[static_cast<std::size_t>(i)].id = i + 1;
		const int code = pthread_create(
			&threads[static_cast<std::size_t>(i)],
			nullptr,
			workerRoutine,
			&starts[static_cast<std::size_t>(i)]);
		if (code != 0) {
			break;
		}
		++created;
	}

	pthread_mutex_lock(&state.mutex);
	while (state.finished < created) {
		pthread_cond_wait(&state.done, &state.mutex);
	}
	pthread_mutex_unlock(&state.mutex);

	joinWorkers(threads, created);
	pthread_cond_destroy(&state.done);
	pthread_mutex_destroy(&state.mutex);

	if (created != threadCount) {
		throw std::runtime_error("Не удалось создать поток");
	}

	return c;
}
