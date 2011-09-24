#include "cmultprocessor.h"
//#include <QDebug>

// ����������� �������� ����������� �� ��������� ��������
CMultProcessor::CMultProcessor( QList<QList<int> > matrix, QStringList nodes,QProgressBar * pb)
{
    count = nodes.size();                   // ������� ������� ��������� �������������� ���������� ������
    matr = matrix;
    vertexes = nodes;
    prB = pb;
    begin = 0;                               // ��������� �
    end = 0;                                 // �������� �������� ��������������� �������
}

// ����������� �������� ����� ������������ �� �������������
CMultProcessor::CMultProcessor(CMultProcessor &proc)
{
    count = proc.count;                      // ���������� ����������
    matr = proc.matr;
    vertexes = proc.vertexes;
    prB = proc.prB;
    begin = proc.begin;                      // ����������� ���������
    end = proc.end;                          // � �������� �������
}

// ����������
CMultProcessor::~CMultProcessor()
{
    matr.clear();
    result.clear();
    vertexes.clear();
}

// �������� ������������
CMultProcessor* CMultProcessor::Clone()
{
    return new CMultProcessor( *this ); // ����� ������������ �����������
}

// ��������� ����������� �������� ���������
void CMultProcessor::GetAlts( QList<int> &alts )
{
    for( int j = 0; j < count; ++j ) // �� ������� ��������� ��������� ����
        if( matr[end][j] == 1 )      // � ������� ����� �������
            alts.push_back( j );          // �� ����� ������� ����
}

// ���������� ��������� ������� � �������
void CMultProcessor::AddLink( int ind )
{
    matr[ end ][ ind ] = 3;         // ����� �� ����� ������� ���� � ����������� ���������� ��������� �������
    matr[ ind ][ end ] = 0;         // �������� ���� (���� �������) ���������

    if( ind != begin )              // ���� ����������� ������� �� ��������� (������������ ����)
        __ClearEx( end, ind );      // ������� ������ ����� ���������� �����

    ind = __GetEx( ind );           // �������� ��������� �������� �� ��������������� ���� �������

    end = ind;                      // ����������� ������� ���������� ������ ������� ����
}

// �������� �� ����������� ����
bool CMultProcessor::IsCycle()
{
    return ( begin == end ); // ������ � ����� ������� ���� �����
}

// �������� �������� ����� �� ������������
bool CMultProcessor::IsValid( bool first)
{
    if( first && begin == end )         // ���� ������ � ����� ������� ���� �����
    {
        QStringList vec;
        __GetList( vec );               // ���������� ������ ������ ������� ����
        if( vec.size() != count + 1 )   // ���� ��� ������ �� ����� ���������� ������ + 1
            return false;               // ��� �� ���������� ����
    }

    // �������� �� ������� ����������� ������ ��� ������ �������
    for( int i = 0; i < count; i++ )
    {
        int sum = 0;
        for( int j = 0; j < count; j++ )
        {
            //          qDebug() <<"matr[i][j] " << matr[i][j] ;
            if (matr[i][j] != 0)
                sum++;
        }

        if( sum == 0 )
        {
            //            qDebug() <<"ishod = 0";
            return false;
        }
    }

    // �������� �� ������� ����������� ������ ��� ������ �������
    for( int j = 0; j < count; ++j )
    {
        int sum = 0;
        for( int i = 0; i < count; ++i )
            if (matr[i][j] != 0)
                sum ++;
        if( sum == 0 )
        {
            //            qDebug() <<"zahod = 0";
            return false;
        }
    }

    return true;
}

// ��������� ���������� �������� �� ��������������� ����
// ���������� � ����������� �������������� ����
// �������� ��������������� ���� � ���������� ����� �����
int CMultProcessor::__GetEx( int from )
{
    for( int j = 0; j < count; ++j ) // �������� ������� ���������
        if( matr[from][j] == 2 )       // ���� �� ��������������� ����� �� ������� from
        {
        matr[from][j] = 3;          // ���� ���� ��� ����������� � �������������� ����
        if( j != begin )              // ���� �������� ������� �� ���������
            return __GetEx( j );        // ��������� �������
        else                          // �����
            return j;                   // ���������� ��
    }
    return from;                      // ���� ������ �� ����� ��������� �� ��� ������ � �����
}

// ��������� ���������� �������� �������
int CMultProcessor::__GetNext( int from )
{
    for( int j = 0; j < count; ++j )  // ����� ��������� ����� �� �������
        if( matr[from][j] == 3 )       // ���������� ��������� �������
            return j;
    return -1;
}

// ���������� ������ ����� ����� ���������� � ������� �����
void CMultProcessor::__ClearEx( int from, int to )
{
    for( int j = 0; j < count; ++j ) // ��� ����� �� ������� - from ���������
        if( matr[from][j] == 1  )
            matr[from][j] = 0;

    for( int i = 0; i < count; ++i )// ��� ����� � ������� - to ���������
        if( matr[i][to] == 1  )
            matr[i][to] = 0;

    __CheckAll();                    // �������� ����������� �����
}

// �������� ����������� �����
void CMultProcessor::__CheckAll()
{
    for( int i = 0; i < count; ++i ) // ����� ������ ������� ����������� ������ ������ �������
    {
        int sum = 0;
        int last = 0;
        for( int j = 0; j < count; ++j )
        {
            if (matr[i][j] == 1)
            {
                sum ++;
                last = j;
            }
        }

        if( sum == 1 )               // ���� ����� ������� �������
        {
            matr[i][last] = 2;           // ��������� �� ��� ����� ���������� ��� �������� �� ��������������� ����
            __ClearEx( i, last );         // ������������ ����� ����� ��������� ������
        }
    }

    for( int j = 0; j < count; ++j ) // ����� ������ ������� ����������� ������ ������ �������
    {
        int sum = 0;
        int last = 0;
        for( int i = 0; i < count; ++i )
        {
            if( matr[i][j] == 1 )
            {
                sum ++;
                last = i;
            }
        }

        if( sum == 1 )              // ���� ����� ������� �������
        {
            matr[last][j] = 2;          // �������� � ��� ����� ���������� ��� �������� �� ��������������� ����
            __ClearEx( last, j );        // ������������ ����� ����� ��������� ������
        }
    }
}

// ��������� ������ �����
void CMultProcessor::__GetList( QList<QString> &vec )
{
    int cur = begin; // ��������� ������� ��������� �������
    vec.push_back( QString::number(cur) ); // ��������� ������� � ������
    do
    {
        cur = __GetNext( cur ); // �������� ��������� �������
        if( cur >= 0 )          // ���� ��� �� �������������
            vec.push_back( QString::number(cur) ); // ��������� �� � ������
    }
    while( cur != begin );  // ��������� ���� ������� �� ������ ��������� (���� ���������)
}

// ������� ������ ������� ������������ �������
QStringList CMultProcessor::ThreadMultiFind()
{
    QList<CMultProcessor *> works;                      // ������ ����������������� (������������ ����)
    works.push_back( this );                            // ��������� ��� � ������
    float last = 100;                                   // ������� �������� ����������
    float progress = 0;                                 // ������� ����������

    while(works.size())                                 // ���� ����������� ���� ������� ���� ����������������
    {
        QList<CMultProcessor *> new_works;              // ���������� ������
        float local_last = last/(works.size()+1);       // ��������� ���������� ������� �������� ����������

        for(int i = 0; i < works.size(); ++i )          // ������������ ��� ��������� � ������ ����������������
        {
            QList<int> alts;                            // ������ �����������
            works[i]->GetAlts(alts);                    // ��������� �����������
            float step = local_last/(alts.size()+1);    // ���������� ���������� ���� ��� �������� ����������

            for( int j = 0; j < alts.size(); ++j )      // ��� ������ ������������
            {
                CMultProcessor *new_proc = works[i]->Clone(); // ��������� ����� ���������������� � ������������
                new_proc->AddLink( alts[j] );           // ����������� ��� ��������� ������� �������
                if( new_proc->IsValid() )               // �������� ������������� �����
                {
                    if( new_proc->IsCycle() )           // ���� ��� ����������� ����
                    {
                        this->addToResult(new_proc);
                        last -= step;                             // ������� �������� ���������� ���������� �� ���
                        progress += step;                         // �������� ������������� �� ���
                        prB->setValue(progress);
                        delete new_proc;                          // ������� ��������������� ���������
                    }
                    else                                        // ����� (����������� ���� �� ������)
                    {
                        new_works.push_back( new_proc );          // ��������������� ����������� � ������� ������
                    }
                }
                else
                    delete new_proc;                            // ������� ��������������� ���������
            }
            last -= step;                                   // ������� �������� ���������� ���������� �� ���
            progress += step;                               // �������� ������������� �� ���
            prB->setValue(progress);
            works[i]->matr.clear();
        }
        for (int i = 0 ; i < works.size() ; i++)
            works[i]->matr.clear();
        works.clear();                                    // ������� ������ �����������������
        for( int i = 0; i < (int)new_works.size(); ++i )// ����������� �������� ������ �����������������
            works.push_back( new_works[i] );                // � ������� ������
    }
    for (int i = 0 ; i < works.size() ; i++)
        works[i]->matr.clear();
    return result;    
}

//
//void CMultProcessor::printMatrix(QList<QList<int> > m)
//{
//    qDebug() << "printMatrix";
//    int tableSize = m.count();
//    qDebug() << "tableSize " << tableSize;
//    QString row;
//    for (int i = 0 ; i < tableSize ; i++)
//    {
//        for (int j = 0 ; j < tableSize ; j++)
//        {
//            row += m[i][j] + "\t";
//        }
//        qDebug() << row;
//        row="";
//    }
//}

void CMultProcessor::addToResult(CMultProcessor *proc)
{
    //    qDebug() << "addToResult";
    QString s = this->vertexes[0];
    int from = 0;
    int kol = 0;
    while (kol != count)
    {
        kol++;
        for( int j = 0; j < count; ++j )
            if( proc->matr[from][j] == 3 )
            {
            from = j;
            break;
        }
        s += this->vertexes[from];
        //        qDebug() << s;
    }
    //    qDebug() << s;
    result << s;
}

