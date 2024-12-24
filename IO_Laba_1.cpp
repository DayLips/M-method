#include <iostream>
#include <vector>
#include <iomanip>
#include "Bludo.h"
#include "Menu.h"
#include <glpk.h>

using namespace std;

const double M = pow(10,5);                        // Штраф

bool Proverka_optimum(vector<double> t);

int main()
{
    
    setlocale(LC_ALL, "RUS");
    Menu data("Menu.txt");
    int max_min = 0;
    // Создание симплекс таблицы
    int n = 2 * data.get_menu()[0].get_param().size() - 1;
    int m = data.get_menu().size() + (n - 1) + (n - 1) / 2 + 1;
    vector<vector<double>> matr(n, vector<double>(m));          // Симплекс таблица
    vector<vector<int>> basis(n - 1, vector<int>(2));           // Вектор для отслеживания базисных векторов и искуств.перем.
    cout << "Если хотите найти максимум или минимум, выберите -1 или 1 соотвественно: ";
    cin >> max_min;
    for(int i = 0, j = 0; i < (n - 1); i += 2, j++)               // Заполнение последнего столбца в симплекс таблице
    {
        cout << "Введите значение ограничения:";
        cin >> matr[i][m - 1];
        double delta = 0.1 * matr[i][m - 1];
        matr[i + 1][m - 1] = matr[i][m - 1] - delta;
        matr[i][m - 1] = matr[i][m - 1] + delta;
    }
    cout << endl;

    for (int i = 0, j = data.get_menu().size(), k = data.get_menu().size() + (n - 1); i < basis.size(); i++) // Заполняем вектор basis
    {
        if (i % 2 == 0) 
        {
            basis[i][0] = j;
            basis[i][1] = 0;
            j += 2;
        }
        else
        {
            basis[i][0] = k;
            basis[i][1] = 1;
            k++;
        }
    }
    for (int j = 0; j < (m - 1); j++)
    {
        for (int i = 0; i < n; i++)
        {
            if (i == (n - 1))                   // Заполнение строки z
            {
                if (j < data.get_menu().size())
                {
                    matr[i][j] = -max_min * data.get_menu()[j].get_param()[(n - 1) / 2];
                }
            }
            if (i < (n - 1))                    // Заполнение векторов блюд, искуственных перемнных и дополнительных
            {
                if (j < data.get_menu().size())
                {
                    if ((i % 2) == 0)
                    {
                        matr[i][j] = data.get_menu()[j].get_param()[i / 2];
                        matr[i + 1][j] = data.get_menu()[j].get_param()[i / 2];
                    }
                }
                else
                {
                    if (basis[i][0] == j)
                    {
                        matr[i][j] = 1;
                    }
                    else if ((j - data.get_menu().size()) == i)
                    {
                        matr[i - 1][j] = -1;
                    }
                }
            }
            
        }
    }
    for (int i = 0; i < basis.size(); i++)
    {
        if (basis[i][1] == 0)
        {
            matr[n - 1][basis[i][0]] = -M;
        }
    }
    for (int i = 0; i < n - 1; i++)     // Приводим симплекс таблицу к применению метода(зануляем коэффициенты у искуст. перем.)
    {
        for (int j = 0; j < m; j++)
        {
            if (basis[i][1] == 0)
            {
                matr[n - 1][j] += M * matr[i][j];
            }
            
        }
    }
    while (!Proverka_optimum(matr[n - 1]))
    {
        vector<int> desision_el = { 0,0 };                              // Находим разрешающий элемент
        double drob = -1;
        for (int j = 0; j < (m - 1); j++)
        {
            if ((matr[n - 1][j] > 0) && (matr[n - 1][j] > drob))
            {
                drob = matr[n - 1][j];
                desision_el[1] = j;
            }
        }
        drob = matr[0][m - 1] / matr[0][desision_el[1]];
        for (int i = 1; i < (n - 1); i++)
        {
            if (matr[i][desision_el[1]] > 0)
            {
                if ((matr[i][m - 1] / matr[i][desision_el[1]]) < drob)
                {
                    drob = matr[i][m - 1] / matr[i][desision_el[1]];
                    desision_el[0] = i;
                }
            }
        }

        if (basis[desision_el[0]][1] == 1)                                  // Удаляем столбец с искуст. перем.
        {
            for (auto& row : matr) 
            {
                if (row.size() > (basis[desision_el[0]][0])) 
                {
                    row.erase(row.begin() + basis[desision_el[0]][0]);
                }
            }
            m -= 1;
            
            for (int i = 0; i < basis.size(); i++)
            {
                if (basis[i][0] > basis[desision_el[0]][0])
                {
                    basis[i][0] -= 1;
                }
            }
            basis[desision_el[0]][0] = desision_el[1];
            basis[desision_el[0]][1] = 0;
        }
        else
        {
            basis[desision_el[0]][0] = desision_el[1];
            basis[desision_el[0]][1] = 0;
        }

        double desision = matr[desision_el[0]][desision_el[1]];
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
            {
                if ((i != desision_el[0]) && (j != desision_el[1]))
                {
                    matr[i][j] = (matr[i][j] * desision - matr[i][desision_el[1]] * matr[desision_el[0]][j]) / desision;
                }
                
            }
        }
        for (int i = 0; i < n; i++)
        {
            if (i != desision_el[0]) matr[i][desision_el[1]] = 0;
        }
        for (int j = 0; j < m; j++)
        {
            if (j != desision_el[1]) matr[desision_el[0]][j] = matr[desision_el[0]][j] / desision;
        }
        matr[desision_el[0]][desision_el[1]] = 1;
    }
    double summ = 0;
    for (int i = 0; i < basis.size(); i++)
    {
        if (basis[i][0] < data.get_menu().size())
        {
            double x = matr[i][m - 1];
            if (i % 2 == 0)
            {
                cout << fixed << setprecision(2) << "Количество " << data.get_menu()[basis[i][0]].get_name() << " = " << x << ", парметры блюда умноженное на кол-во =";
                for (int j = 0; j < data.get_menu()[basis[i][0]].get_param().size(); j++)
                {
                    cout << " " << fixed << setprecision(2) << x * data.get_menu()[basis[i][0]].get_param()[j] << ",";
                    if (j == data.get_menu()[basis[i][0]].get_param().size() - 1) summ += x * data.get_menu()[basis[i][0]].get_param()[j];
                }
            }
            else
            {
                cout << fixed << setw(6) << setprecision(2) << "Количество " << data.get_menu()[basis[i][0]].get_name() << " = " << x << ", парметры блюда умноженное на кол-во =";
                for (int j = 0; j < data.get_menu()[basis[i][0]].get_param().size(); j++)
                {
                    cout << " " << fixed << setprecision(2) << x * data.get_menu()[basis[i][0]].get_param()[j] << ",";
                    if (j == data.get_menu()[basis[i][0]].get_param().size() - 1) summ += x * data.get_menu()[basis[i][0]].get_param()[j];
                }
            }
            cout << endl;
            
        }
    }
    cout << endl << "Итоговая цена = " << summ << " рублей " << endl;

    
}

//int main() {
//    // Создаем задачу
//    setlocale(LC_ALL, "RU");
//    glp_prob* lp;
//    lp = glp_create_prob();
//    glp_set_obj_dir(lp, GLP_MAX); // Устанавливаем максимизацию
//
//    // Определяем количество переменных
//    glp_add_cols(lp, 2); // 2 переменные
//    glp_set_col_name(lp, 1, "x1");
//    glp_set_col_name(lp, 2, "x2");
//
//    // Устанавливаем коэффициенты целевой функции
//    glp_set_obj_coef(lp, 1, 3); // z = 3*x1
//    glp_set_obj_coef(lp, 2, 5); // z = 5*x2
//
//    // Определяем количество ограничений
//    glp_add_rows(lp, 2); // 2 ограничения
//    glp_set_row_name(lp, 1, "constraint1");
//    glp_set_row_bnds(lp, 1, GLP_UP, 0.0, 10.0); // Ограничение 1: x1 + 2*x2 ≤ 10
//
//    glp_set_row_name(lp, 2, "constraint2");
//    glp_set_row_bnds(lp, 2, GLP_UP, 0.0, 15.0); // Ограничение 2: 3*x1 + x2 ≤ 15
//
//    // Задаем коэффициенты ограничений
//    int ia[1 + 4], ja[1 + 4]; // Индексы от 1 до 4
//    double ar[1 + 4];
//
//    // Ограничение 1: x1 + 2*x2
//    ia[1] = 1; ja[1] = 1; ar[1] = 1.0; // x1
//    ia[2] = 1; ja[2] = 2; ar[2] = 2.0; // 2*x2
//
//    // Ограничение 2: 3*x1 + x2
//    ia[3] = 2; ja[3] = 1; ar[3] = 3.0; // 3*x1
//    ia[4] = 2; ja[4] = 2; ar[4] = 1.0; // x2
//
//    // Загружаем ограничения в задачу
//    glp_load_matrix(lp, 4, ia, ja, ar); // 4 - количество ненулевых элементов
//
//    // Решаем задачу
//    glp_simplex(lp, NULL);
//
//    // Получаем и выводим результат
//    std::cout << "Целевая функция = " << glp_get_obj_val(lp) << std::endl;
//    std::cout << "x1 = " << glp_get_col_prim(lp, 1) << std::endl;
//    std::cout << "x2 = " << glp_get_col_prim(lp, 2) << std::endl;
//
//    // Освобождаем память
//    glp_delete_prob(lp);
//    return 0;
//}



bool Proverka_optimum(vector<double> t)
{
    bool flag = true;
    for (int i = 0; i < (t.size() - 1); i++)
    {
        if ((t[i] / 1.0) > 0)
        {
            flag = false;
            break;
        }
    }
    return flag;
}
