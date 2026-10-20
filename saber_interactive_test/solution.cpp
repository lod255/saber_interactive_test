/*
 * Тестовое задание C++, Saber Interactive.
 * ФИО: Тест тест (как указано соискателем).
 * Дата выполнения: 20.10.2026.
 * Примерное время: около 5 минут, включая проверку.
 * Сборка: clang++ -std=c++11 -Wall -Wextra -Wpedantic solution.cpp -o solution
 */
#include <climits>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Задача 1. Читаем представление объекта через unsigned char: это разрешено
// стандартом; не предполагаем конкретное кодирование отрицательных чисел.
// Выводим все биты, включая ведущие нули: старший байт первым на обычных
// little-endian системах. На big-endian системах байты уже в нужном порядке.
void PrintBinary(int value)
{
    const unsigned int marker = 1;
    const bool littleEndian = *reinterpret_cast<const unsigned char*>(&marker) == 1;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);
    for (std::size_t i = 0; i < sizeof(value); ++i)
    {
        const std::size_t index = littleEndian ? sizeof(value) - 1 - i : i;
        for (int bit = CHAR_BIT - 1; bit >= 0; --bit)
        {
            std::putchar(((bytes[index] >> bit) & 1U) ? '1' : '0');
        }
    }
    std::putchar('\n');
}

// Задача 2. Изменение на месте: O(n) времени и O(1) дополнительной памяти.
// str должен указывать на доступную для записи строку с завершающим нулём.
void RemoveDups(char* str)
{
    if (str == nullptr || *str == '\0')
    {
        return;
    }
    char* write = str;
    for (char* read = str + 1; *read != '\0'; ++read)
    {
        if (*read != *write)
        {
            *++write = *read;
        }
    }
    *++write = '\0';
}

// Структура из условия оставлена без изменений.
struct ListNode
{
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;
};

namespace Binary
{
// Формат: 8 байт сигнатуры с версией, uint64 количества узлов, затем для
// каждого узла uint64 индекса rand, uint64 длины, байты строки без терминатора.
// Числа записываются младшим байтом вперёд, nullptr обозначается UINT64_MAX.
// prev/next восстанавливаются по порядку записей; адреса в файл не попадают.
const unsigned char signature[] = {'S', 'L', 'I', 'S', 'T', 1, 0, 0};
const std::uint64_t nullIndex = std::numeric_limits<std::uint64_t>::max();
static_assert(CHAR_BIT == 8, "Для формата файла требуется восьмибитный байт");

void Write(FILE* file, const void* data, std::size_t size)
{
    if (size != 0 && std::fwrite(data, 1, size, file) != size)
    {
        throw std::runtime_error("Ошибка записи файла");
    }
}

void Read(FILE* file, void* data, std::size_t size)
{
    if (size != 0 && std::fread(data, 1, size, file) != size)
    {
        throw std::runtime_error("Ошибка чтения или обрезанный файл");
    }
}

void WriteNumber(FILE* file, std::uint64_t value)
{
    unsigned char bytes[8];
    for (unsigned int i = 0; i < 8; ++i)
    {
        bytes[i] = static_cast<unsigned char>(value >> (i * 8));
    }
    Write(file, bytes, sizeof(bytes));
}

std::uint64_t ReadNumber(FILE* file)
{
    unsigned char bytes[8];
    Read(file, bytes, sizeof(bytes));
    std::uint64_t value = 0;
    for (unsigned int i = 0; i < 8; ++i)
    {
        value |= static_cast<std::uint64_t>(bytes[i]) << (i * 8);
    }
    return value;
}
}

class List
{
public:
    List() = default;
    ~List()
    {
        Clear();
    }

    // Список владеет узлами; поверхностное копирование привело бы к double free.
    List(const List&) = delete;
    List& operator=(const List&) = delete;

    ListNode* Append(const std::string& data)
    {
        if (count == INT_MAX)
        {
            throw std::length_error("Слишком много узлов");
        }
        std::unique_ptr<ListNode> node(new ListNode);
        node->data = data;
        node->prev = tail;
        if (tail != nullptr)
        {
            tail->next = node.get();
        }
        else
        {
            head = node.get();
        }
        tail = node.release();
        ++count;
        return tail;
    }

    const ListNode* Head() const { return head; }
    int Size() const { return count; }

    // O(n log n + B) времени, O(n) дополнительной памяти.
    // Исключения сообщают об ошибках; FILE принадлежит вызывающему коду.
    // Вызывающий код обязан проверить также fflush/fclose (ошибки буфера).
    void Serialize(FILE* file)
    {
        if (file == nullptr)
        {
            throw std::invalid_argument("Передан пустой FILE*");
        }
        // map даёт гарантированное O(n log n), включая проверку чужих rand.
        std::map<const ListNode*, std::uint64_t> indices;
        const ListNode* previous = nullptr;
        std::uint64_t index = 0;
        for (const ListNode* node = head; node != nullptr; node = node->next)
        {
            if (index >= static_cast<std::uint64_t>(count) ||
                node->prev != previous || !indices.emplace(node, index).second)
            {
                throw std::logic_error("Нарушена структура списка");
            }
            previous = node;
            ++index;
        }
        if (index != static_cast<std::uint64_t>(count) || previous != tail)
        {
            throw std::logic_error("Неверные count или tail");
        }
        for (const ListNode* node = head; node != nullptr; node = node->next)
        {
            if (node->rand != nullptr && indices.find(node->rand) == indices.end())
            {
                throw std::logic_error("rand указывает за пределы списка");
            }
        }
        Binary::Write(file, Binary::signature, sizeof(Binary::signature));
        Binary::WriteNumber(file, index);
        for (const ListNode* node = head; node != nullptr; node = node->next)
        {
            Binary::WriteNumber(file, node->rand == nullptr ? Binary::nullIndex : indices.at(node->rand));
            Binary::WriteNumber(file, static_cast<std::uint64_t>(node->data.size()));
            Binary::Write(file, node->data.data(), node->data.size());
        }
    }

    // O(n + B) времени и O(n + B) памяти, B — общий размер строк.
    // До полного успешного чтения текущий список не меняется.
    void Deserialize(FILE* file)
    {
        if (file == nullptr)
        {
            throw std::invalid_argument("Передан пустой FILE*");
        }
        unsigned char signature[sizeof(Binary::signature)];
        Binary::Read(file, signature, sizeof(signature));
        for (std::size_t i = 0; i < sizeof(signature); ++i)
        {
            if (signature[i] != Binary::signature[i])
            {
                throw std::runtime_error("Неизвестный формат или версия списка");
            }
        }
        const std::uint64_t size = Binary::ReadNumber(file);
        if (size > INT_MAX)
        {
            throw std::length_error("Количество узлов превышает INT_MAX");
        }
        List restored;
        std::vector<ListNode*> nodes;
        std::vector<std::uint64_t> randomIndices;
        // Не выделяем память сразу по недоверенному размеру из заголовка.
        for (std::uint64_t i = 0; i < size; ++i)
        {
            const std::uint64_t randomIndex = Binary::ReadNumber(file);
            if (randomIndex != Binary::nullIndex && randomIndex >= size)
            {
                throw std::runtime_error("Некорректный индекс rand");
            }
            std::uint64_t remaining = Binary::ReadNumber(file);
            std::string data;
            if (remaining > data.max_size())
            {
                throw std::length_error("Строка слишком велика");
            }
            char buffer[4096];
            while (remaining != 0)
            {
                const std::size_t chunk = remaining < sizeof(buffer)
                    ? static_cast<std::size_t>(remaining) : sizeof(buffer);
                Binary::Read(file, buffer, chunk);
                data.append(buffer, chunk);
                remaining -= chunk;
            }
            nodes.push_back(restored.Append(data));
            randomIndices.push_back(randomIndex);
        }
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            nodes[i]->rand = randomIndices[i] == Binary::nullIndex
                ? nullptr : nodes[static_cast<std::size_t>(randomIndices[i])];
        }
        std::swap(head, restored.head);
        std::swap(tail, restored.tail);
        std::swap(count, restored.count);
        // Деструктор restored освобождает прежний список, в том числе при ошибках.
    }

private:
    void Clear() noexcept
    {
        while (head != nullptr)
        {
            ListNode* next = head->next;
            delete head;
            head = next;
        }
        tail = nullptr;
        count = 0;
    }

    ListNode* head = nullptr;
    ListNode* tail = nullptr;
    int count = 0;
};

// Пример запуска. Для подключения к другому приложению: -DSOLUTION_NO_MAIN.
#ifndef SOLUTION_NO_MAIN
int main()
{
    try
    {
        PrintBinary(-42);
        char data[] = "AAA BBB   AAA";
        RemoveDups(data);
        std::printf("%s\n", data);

        List source;
        ListNode* first = source.Append("Первый");
        ListNode* second = source.Append(std::string("A\0B", 3));
        ListNode* third = source.Append("");
        first->rand = third;
        second->rand = second;
        third->rand = first;
        std::unique_ptr<FILE, int (*)(FILE*)> file(std::tmpfile(), &std::fclose);
        if (!file)
        {
            throw std::runtime_error("Не удалось открыть временный файл");
        }
        source.Serialize(file.get());
        if (std::fflush(file.get()) != 0 || std::fseek(file.get(), 0, SEEK_SET) != 0)
        {
            throw std::runtime_error("Ошибка подготовки файла к чтению");
        }
        List restored;
        restored.Deserialize(file.get());
        std::printf("Восстановлено узлов: %d\n", restored.Size());
        const ListNode* node = restored.Head();
        std::printf("Бинарная строка: %zu байта; обратная ссылка: %s\n",
                    node->next->data.size(), node->next->prev == node ? "да" : "нет");
        std::printf("Ссылки rand восстановлены: %s\n",
                    node->rand == node->next->next &&
                    node->next->rand == node->next &&
                    node->next->next->rand == node ? "да" : "нет");
        FILE* closing = file.release();
        if (std::fclose(closing) != 0)
        {
            throw std::runtime_error("Ошибка закрытия файла");
        }
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
    return 0;
}
#endif
