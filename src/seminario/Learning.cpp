#include "Learning.h"
#include <dlib/svm.h>
#include "csvParser.h"

using namespace dlib;
using seminario::Learning;

typedef matrix < double > SingleSample;
typedef std::vector< SingleSample > SetSample;
typedef std::vector< double > SetLabels;
typedef vector_normalizer< SingleSample > SetNormalizer;
typedef radial_basis_kernel<SingleSample> KernelType;
typedef decision_function<KernelType> DecisionFunction;
typedef normalized_function<DecisionFunction> FunctionType;

const SString Learning::filesInputDirectory = "/home/kevin/ddpc/seminarioJs/.meteor/local/build/programs/server/assets/app/uploads/Text/";
const SString Learning::filesOutputDirectory = "./savedFiles/";
std::unique_ptr< Learning > Learning::m_instance= nullptr;

Learning::Learning()
{}

Learning::~Learning()
{}

bool Learning::getFileAndExist( const Value& params , string& filename )
{
    if( !params.IsArray() )
    {
        cout << "params error format" << endl;
        return false;
    }
    if( static_cast< int >( params.Size() ) != 1 )
    {
        cout << "params error size" << endl;
        return false;
    }
    const Value& fileValue=params[ 0 ];
    if( !fileValue.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filename = fileValue.GetString();
    return true;
}

bool Learning::getFileAndExist( const Value& params , SString& filenameTrained , SString& filename )
{
    if( !params.IsArray() )
    {
        cout << "params error format" << endl;
        return false;
    }
    if( static_cast< int >( params.Size() ) != 2 )
    {
        cout << "params error size" << endl;
        return false;
    }
    const Value& fileValueTrained=params[ 0 ];
    if( !fileValueTrained.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filenameTrained = fileValueTrained.GetString();

    const Value& fileValue=params[ 1 ];
    if( !fileValue.IsString() )
    {
        cout << "error FileId" << endl;
        return false;
    }
    filename = fileValue.GetString();

    return true;
}

bool readSamples( const SString& filename , SetSample& setSample , SetLabels& setLabels , unsigned int& countFields , unsigned int& countSamples )
{
    csvParser csv;
    if( !csv.openFile( filename ) )
    {
        return false;
    }
    while( true )
    {
        SString line=csv.nextLine();
        if( line.empty() )
        {
            break;
        }
        countSamples++;
        auto tokens = csv.split( line , ' ' );
        countFields = tokens.size() - 1;
        SingleSample sample( countFields , 1 );
        for( int j = 0 ; j < countFields ; j++ )    
        {
            sample( j ) = stof( tokens[ j ] );
        }
        setSample.push_back( sample );
        const double label = stof( tokens[ countFields ] ) ;
        if( label == 0 )
        {
            setLabels.push_back( -1 );
        }
        else
        {
            setLabels.push_back( +1 );
        }
    }
    return true;   
}

bool readSample( const SString& filename , SingleSample& sample )
{
    csvParser csv;
    if( !csv.openFile( filename ) )
    {
        return false;
    }
    SString line = csv.nextLine();
    if( line.empty() )
    {
        return false;
    }
    auto tokens = csv.split( line , ' ' );
    const unsigned int countFields = tokens.size();
    SingleSample temp( countFields , 1 );
    for( int i=0 ; i< countFields ; i++ )    
    {
        temp( i ) = stof( tokens[ i ] );
    }
    sample = temp;
    return true;
}

void Learning::train( const Value& params , SString& output )
{
    output="ggwp";
    if( true )
    {
        return;
    }
    SString filename;
    if( !getFileAndExist( params , filename ) )
    {
        output="{ error : \"no file\" }";
        return;
    }
    SetSample setSample;
    SetLabels setLabels;
    unsigned int countFields = 0;
    unsigned int countSamples = 0;
    // leyendo data
    if ( !readSamples( filesInputDirectory + filename +".csv" , setSample , setLabels , countFields , countSamples ) )
    {
        output="{ error : \"file error\" }";
        return;
    }
    // Normalizando la data
    SetNormalizer normalizer;
    normalizer.train(setSample);
    for ( unsigned long i = 0; i < countSamples ; ++i)
    {
        setSample[i] = normalizer(setSample[i]); 
    }
    randomize_samples(setSample, setLabels);
    const double max_nu = maximum_nu(setLabels);
    cout << max_nu << endl;
    // cross validation
    svm_nu_trainer< KernelType > trainer;
    cout << "starting cross validation" << endl;
    for (double gamma = 0.00001; gamma <= 1; gamma *= 5)
    {
        for (double nu = 0.00001; nu < max_nu; nu *= 5)
        {
            trainer.set_kernel( KernelType( gamma ) );
            trainer.set_nu(nu);
            cout << "gamma: " << gamma << "    nu: " << nu;
            cout << "cross validation accuracy: " << cross_validate_trainer( trainer , setSample , setLabels , 3 );
        }
    }
    // seleccionando mejor funcion
    FunctionType learned_function;
    learned_function.normalizer = normalizer;
    learned_function.function = trainer.train( setSample, setLabels );
    // guardando data
    serialize( filesOutputDirectory + filename +".dat") << learned_function;
}

void Learning::predict( const Value& params , string& output )
{
    SString filenameTrained;
    SString filename;
    if( !getFileAndExist( params , filenameTrained , filename ) )
    {
        output="{ error : \"no file\" }";
        return;
    }
    SingleSample sample;
    if ( !readSample( filesInputDirectory + filename +".csv" , sample ) )
    {
        output="{ error : \"file error\" }";
        return;
    }
    FunctionType learned_function;
    deserialize( filesOutputDirectory + filenameTrained +".dat" ) >> learned_function;
    auto result = learned_function(sample);
    std::ostringstream s;
    s << "{ result : \"" << result << "\" }";
    output = s.str();
}
