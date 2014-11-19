#include <cstdlib>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <mmsystem.h>
#include <mpi.h>
#include "geometryOfHulls.h"
#include "point.h"
#include "line.h"
#include <vector>
//Подключаем библиотеку
#pragma comment(lib,"winmm.lib")
//Определяем путь файла
#define NAME_OF_FILE "C:\\mpi_3lab_parallel\\linearseparability.txt"
#define NAME_OF_FILE_MANUAL_INPUT "C:\\mpi_3lab_parallel\\manualinput.txt"

using namespace std;

int main (int argc, char *argv[]) {
	int countOfPoints1=0, countOfPoints2=0;
	fstream fileStream;
	int procNum, procRank;
	MPI_Status status;
	DWORD timeStart;
	srand(1);
	//Чтение из файла или генерация рандомно
	int mode;
	if (argc>0)
		mode = atoi(argv[1]);

	MPI_Init ( &argc, &argv );
	MPI_Comm_size ( MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank ( MPI_COMM_WORLD, &procRank);
	//Изначальные наборы вершин
	vector<point> s1;
	vector<point> s2;
	//Выпуклые оболочки (номера в s1 и s2)
	vector<int> hull1;
	vector<int> hull2;
	//Координаты оболочки
	vector<point> hull2Points;
	vector<point> hull1Points;
	//Массивы с размерами точек ниже и выше прямой (в 0 процессе)
	vector<int> bSizeWithNull(procNum);
	vector<int> tSizeWithNull(procNum);
	//Смещение для передачи точек выше и ниже прямой
	vector<int> sendVertex = vector<int>(procNum);
	vector<int> sendDispls = vector<int>(procNum);
	//Переменные для передачи размеров 0 процессу
	int b;
	int t;
	//Сами массивы с индексами точек
	vector<int> top;
	vector<int> bot;
	//Массивы вершин снизу и сверху разделяющей прямой
	vector<int> topPoints;
	vector<int> botPoints;
	vector<int> topDispls;
	vector<int> botDispls;
	//Объект для доступа к методам
	GeometryOfHulls geom = GeometryOfHulls(procNum);
	//Номера вершин разделяющего отрезка
	vector<int> lr;
	//Контейнер для передачи lr
	vector<line> separateLine(1);
	//Оболочка, разбитая на отрезки (для пересечения)
	vector<line> vline2;
	vector<line> vline1;
	//Размер массива с отрезками для передачи процессам (необходимо узнать сколько памяти выделять)
	int vlineSize1 = 0;
	//Переменные каждого процесса для записи в них соответствующих ошибок
	int nestingEach2= 0;
	int nestingEach1= 0;
	int collision = 0;
	//Переменная, которая собирает ошибки функцией Reduce
	int error =0;
	int errorNesting1 =0;
	int errorNesting2 =0;
	//Тип данных point
	MPI_Datatype pointType;
    MPI_Type_contiguous(2, MPI_DOUBLE, &pointType);
    MPI_Type_commit(&pointType);
    //Тип данных line
    MPI_Datatype lineType;
    MPI_Aint offsets[2], oldtypes[2], blockcounts[2], extent;
    offsets[0] = 0;
	oldtypes[0] = pointType;
	blockcounts[0] = 2;
	MPI_Type_extent(pointType, &extent);
	offsets[1] = 2 * extent;
	oldtypes[1] = MPI_DOUBLE;
	blockcounts[1] = 3;
    MPI_Type_struct(2, blockcounts, offsets, oldtypes, &lineType);
    MPI_Type_commit(&lineType);
	if (procRank == 0) {
	    //Запускаем таймер
	    if (mode == 1) {
    	    if (argc>2){
    	    	countOfPoints1 = atoi(argv[2]);
    	    	countOfPoints2 = atoi(argv[3]);
    	    }
    	   	s1 = vector<point> (countOfPoints1);
    	   	s2 = vector<point> (countOfPoints2);
   	   	    //Открываем файл для записи
   	   	    fileStream.open(NAME_OF_FILE, ios::in | ios::out | ios::binary | ios::trunc);
   	   	    if (fileStream.fail()){
   	   	    	cout<<"Error opening "<<NAME_OF_FILE<<endl;
   	   	    	exit(1);
   	   	    }
       	   	//Инициализируем случайными значениями
       	   	for (int i=0; i<countOfPoints1; i++)
       	   	  	s1[i].setPosition(static_cast<double>((1+rand()%100)*0.1), static_cast<double>((1+rand()%100)*0.1));
       	   	    for (int i=0; i<countOfPoints2; i++)
       	   	    	s2[i].setPosition(static_cast<double>((1+rand()%100)*0.1), static_cast<double>((1+rand()%100)*0.1));
       	    //Записываем матрицу в файл
       	    fileStream<<"Координаты первого множества";
       	    fileStream<<"\r\n";
       	    for (int i=0; i<countOfPoints1; i++){
       	    	fileStream<<s1[i].getX()<<" ";
       	    	fileStream<<s1[i].getY()<<" ";
       	    	fileStream<<"\r\n";
       	    }
   	   	    fileStream<<"Координаты второго множества";
   	   	    fileStream<<"\r\n";
   	   	    for (int i=0; i<countOfPoints2; i++){
   	   	    	fileStream<<s2[i].getX()<<" ";
   	   	    	fileStream<<s2[i].getY()<<" ";
   	   	    	fileStream<<"\r\n";
   	   	    }

    	   	fileStream.close();
    	} else if (mode == 2) {
    	    //Открываем файл для чтения
    		fileStream.open(NAME_OF_FILE_MANUAL_INPUT);
    		if (fileStream.fail()){
    			cout<<"Error opening "<<NAME_OF_FILE_MANUAL_INPUT<<endl;
    			exit(1);
    		}
    		fileStream>>countOfPoints1;
    		s1 = vector<point>(countOfPoints1);
    		for (int i=0; i<countOfPoints1; i++) {
    			double valueFromFile;
    			fileStream>>valueFromFile;
    			s1[i].setPosition(valueFromFile, s1[i].getY());
    			fileStream>>valueFromFile;
    			s1[i].setPosition(s1[i].getX(), valueFromFile);
    		}
    		fileStream>>countOfPoints2;
    		s2 = vector<point>(countOfPoints2);
    		for (int i=0; i<countOfPoints2; i++) {
    			double valueFromFile;
    			fileStream>>valueFromFile;
    			s2[i].setPosition(valueFromFile, s2[i].getY());
    			fileStream>>valueFromFile;
    			s2[i].setPosition(s2[i].getX(), valueFromFile);
    		}
    		fileStream.close();
	    }
	}

	if (procRank == 0) {
	    timeStart = timeGetTime();
	    lr = geom.getSeparatingLine(s1);
	    separateLine[0] = line(s1[lr[0]], s1[lr[1]]);
	    geom.distributeForProcesses(s1, sendVertex, sendDispls);
	}
	//Передаем массивы смещений и длины
	MPI_Bcast(&sendVertex[0], sendVertex.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&sendDispls[0], sendDispls.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&separateLine[0], separateLine.size(), lineType, 0, MPI_COMM_WORLD);
	MPI_Bcast(&countOfPoints1, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//Выделяем память под вершины (общий размер равен кол-ву вершин)
	vector<point> recBuffer(sendVertex[procRank]);
	//Передаем каждому процессу соответствующий кусок
	MPI_Scatterv(&s1[0], &sendVertex[0], &sendDispls[0], pointType, &recBuffer[0], sendVertex[procRank], pointType, 0, MPI_COMM_WORLD);
	geom.initializeTopAndBotSets(recBuffer, sendDispls[procRank], separateLine[0], top, bot);
	b = bot.size();
	t = top.size();
	MPI_Gather(&b, 1, MPI_INT, &bSizeWithNull[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(&t, 1, MPI_INT, &tSizeWithNull[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (procRank==0){
		int bCount = 0;
		int tCount = 0;
		for (int i=0; i<bSizeWithNull.size(); i++)
			bCount+=bSizeWithNull[i];
		for (int i=0; i<tSizeWithNull.size(); i++)
			tCount+=tSizeWithNull[i];
		botPoints = vector<int>(bCount);
		topPoints = vector<int>(tCount);
		int sum=0;
		botDispls = vector<int>(bSizeWithNull.size());
		for (int i=0; i<bSizeWithNull.size(); i++)  {
			botDispls[i] = sum;
			sum+=bSizeWithNull[i];
		}
		sum=0;
		topDispls = vector<int>(tSizeWithNull.size());
		for (int i=0; i<tSizeWithNull.size(); i++)  {
			topDispls[i] = sum;
			sum+=tSizeWithNull[i];
		}
	}

	MPI_Gatherv(&bot[0], bot.size(), MPI_INT, &botPoints[0], &bSizeWithNull[0], &botDispls[0], MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gatherv(&top[0], top.size(), MPI_INT, &topPoints[0], &tSizeWithNull[0], &topDispls[0], MPI_INT, 0, MPI_COMM_WORLD);

	if (procRank==0){
	geom.quickHull(s1, hull1, lr[0], lr[1], topPoints);
	geom.quickHull(s1, hull1, lr[1], lr[0], botPoints);
	for (int i=0; i<hull1.size(); i++)
		cout<<hull1[i]<<" <1> "<<endl;
	}
	/*Тоже самое для 2 оболочки*/
	sendVertex = vector<int>(procNum);
	sendDispls = vector<int>(procNum);
	if (procRank == 0) {
	    lr = geom.getSeparatingLine(s2);
	    separateLine[0] = line(s2[lr[0]], s2[lr[1]]);
	    geom.distributeForProcesses(s2, sendVertex, sendDispls);
	    bSizeWithNull = vector<int>(procNum);
	    tSizeWithNull = vector<int>(procNum);
	}

	//Передаем массивы смещений и длины
	MPI_Bcast(&sendVertex[0], sendVertex.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&sendDispls[0], sendDispls.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&separateLine[0], separateLine.size(), lineType, 0, MPI_COMM_WORLD);
	MPI_Bcast(&countOfPoints2, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//Выделяем память под вершины (общий размер равен кол-ву вершин)
	recBuffer = vector<point> (sendVertex[procRank]);
	//Передаем каждому процессу соответствующий кусок
	MPI_Scatterv(&s2[0], &sendVertex[0], &sendDispls[0], pointType, &recBuffer[0], sendVertex[procRank], pointType, 0, MPI_COMM_WORLD);
	bot.clear();
	top.clear();
	geom.initializeTopAndBotSets(recBuffer, sendDispls[procRank], separateLine[0], top, bot);
	b = bot.size();
	t = top.size();
	MPI_Gather(&b, 1, MPI_INT, &bSizeWithNull[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(&t, 1, MPI_INT, &tSizeWithNull[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (procRank==0){
		int bCount = 0;
		int tCount = 0;
		for (int i=0; i<bSizeWithNull.size(); i++)
			bCount+=bSizeWithNull[i];
		for (int i=0; i<tSizeWithNull.size(); i++)
			tCount+=tSizeWithNull[i];
		botPoints = vector<int>(bCount);
		topPoints = vector<int>(tCount);
		int sum=0;
		botDispls = vector<int>(bSizeWithNull.size());
		for (int i=0; i<bSizeWithNull.size(); i++)  {
			botDispls[i] = sum;
			sum+=bSizeWithNull[i];
		}
		sum=0;
		topDispls = vector<int>(tSizeWithNull.size());
		for (int i=0; i<tSizeWithNull.size(); i++)  {
			topDispls[i] = sum;
			sum+=tSizeWithNull[i];
		}
	}
	MPI_Gatherv(&bot[0], bot.size(), MPI_INT, &botPoints[0], &bSizeWithNull[0], &botDispls[0], MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gatherv(&top[0], top.size(), MPI_INT, &topPoints[0], &tSizeWithNull[0], &topDispls[0], MPI_INT, 0, MPI_COMM_WORLD);
	sendVertex = vector<int>(procNum);
	sendDispls = vector<int>(procNum);
	if (procRank==0){
	geom.quickHull(s2, hull2, lr[0], lr[1], topPoints);
	geom.quickHull(s2, hull2, lr[1], lr[0], botPoints);
		for (int i=0; i<hull2.size(); i++)
		cout<<hull2[i]<<" <2> "<<endl;
	//Чтобы не рассматривать замкнутый вектор
	hull2.push_back(hull2[0]);
	hull1.push_back(hull1[0]);
	vline2 = vector<line>(hull2.size()-1);
	vline1 = vector<line>(hull1.size()-1);
	//Чтобы не передавать оболочку нам нужно узнать размерность вектора оболочки 1 множ-ва
	vlineSize1 = vline1.size();
	geom.getLinesForProcesses(s2, hull2, vline2);
	geom.getLinesForProcesses(s1, hull1, vline1);
	geom.distributeForProcesses(vline2, sendVertex, sendDispls);
	}
	//Передаем массивы смещений и длины
	MPI_Bcast(&sendVertex[0], sendVertex.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&sendDispls[0], sendDispls.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&vlineSize1, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (procRank!=0) {
		vline1 = vector<line>(vlineSize1);
	}
	MPI_Bcast(&vline1[0], vline1.size(), lineType, 0, MPI_COMM_WORLD);
	//Выделяем память под вершины (общий размер равен кол-ву вершин)
	vector<line> recBufferLine(sendVertex[procRank]);
	//Передаем каждому процессу соответствующий кусок
	MPI_Scatterv(&vline2[0], &sendVertex[0], &sendDispls[0], lineType, &recBufferLine[0], sendVertex[procRank], lineType, 0, MPI_COMM_WORLD);
	for (int i=0; i<vline1.size(); i++) {
		for (int j=0; j<recBufferLine.size(); j++) {
			if (geom.intersection(vline1[i], recBufferLine[j], procRank) == true) {
				collision++;
			}
		}
	}
	/*Узнаем содержится ли одна в другой*/
	sendVertex = vector<int>(procNum);
	sendDispls = vector<int>(procNum);
	if (procRank == 0) {
		hull2.pop_back();
		hull2Points = vector<point>(hull2.size());
		for (int i = 0; i<hull2.size(); i++) {
			hull2Points[i] = s2[hull2[i]];
		}
		geom.distributeForProcesses(hull2Points, sendVertex, sendDispls);

	}
	MPI_Bcast(&sendVertex[0], sendVertex.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&sendDispls[0], sendDispls.size(), MPI_INT, 0, MPI_COMM_WORLD);
	//vline2 - это отрезки из 2 оболчки, vline1 - из 1 оболочки
	recBuffer = vector<point>(sendVertex[procRank]);
	MPI_Scatterv(&hull2Points[0], &sendVertex[0], &sendDispls[0], pointType, &recBuffer[0], sendVertex[procRank], pointType, 0, MPI_COMM_WORLD);
	for (int i=0; i<recBuffer.size(); i++)
		nestingEach2+=geom.nesting(recBuffer[i], vline1);
	/*Случай, если их поменять местами*/
	sendVertex = vector<int>(procNum);
	sendDispls = vector<int>(procNum);
	if (procRank == 0) {
		hull1.pop_back();
		hull1Points = vector<point>(hull1.size());
		for (int i = 0; i<hull1.size(); i++) {
			hull1Points[i] = s1[hull1[i]];
		}
		geom.distributeForProcesses(hull1Points, sendVertex, sendDispls);
	}
	MPI_Bcast(&sendVertex[0], sendVertex.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&sendDispls[0], sendDispls.size(), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&vline2[0], vline2.size(), lineType, 0, MPI_COMM_WORLD);
	recBuffer = vector<point>(sendVertex[procRank]);
	MPI_Scatterv(&hull1Points[0], &sendVertex[0], &sendDispls[0], pointType, &recBuffer[0], sendVertex[procRank], pointType, 0, MPI_COMM_WORLD);
	for (int i=0; i<recBuffer.size(); i++)
		nestingEach1+=geom.nesting(recBuffer[i], vline2);
	//Собираем результаты в 0 процессе
	MPI_Reduce(&collision, &error, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&nestingEach2, &errorNesting2, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&nestingEach1, &errorNesting1, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (procRank==0) {
	if (errorNesting2==hull2.size())
	{
		cout<<"NESTING"<<endl;
	}
	if (errorNesting1==hull1.size())
	{
		cout<<"NESTING"<<endl;
	}

	if (error>0) cout<<"SEPARATE"<<endl;
	else cout<<"NOT SEPARATE"<<endl;
    DWORD timeEnd=timeGetTime();
	DWORD time=timeEnd-timeStart;
	//Выводим время выполнения
	cout<<"Elapsed time: "<< (double) time/1000<<endl;
	}
	MPI_Finalize();
	return 0;
}