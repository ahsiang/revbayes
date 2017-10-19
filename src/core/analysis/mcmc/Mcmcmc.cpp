#include "Mcmcmc.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RlUserInterface.h"
#include "RbConstants.h"
#include "RbException.h"

#include <iomanip>
#include <iostream>
#include <vector>
#include <cmath>

#ifdef RB_MPI
#include <mpi.h>
#endif

using namespace RevBayesCore;

Mcmcmc::Mcmcmc(const Model& m, const RbVector<Move> &mv, const RbVector<Monitor> &mn, std::string sT, size_t nc, size_t si, double dt, bool th, std::string sm) : MonteCarloSampler( ),
num_chains(nc),
schedule_type(sT),
current_generation(0),
burnin_generation(0),
generation(0),
swap_interval(si),
swap_interval2(0),
active_chain_index( 0 ),
delta( dt ),
tune_heat(th),
useNeighborSwapping(true),
useRandomSwapping(false)
{
    
    // initialize container sizes
    chains = std::vector<Mcmc*>(num_chains, NULL);
    chain_values.resize(num_chains, 0.0);
    chain_heats.resize(num_chains, 0.0);
    pid_per_chain.resize(num_chains, 0);
    heat_ranks.resize(num_chains, 0);
    heat_temps.resize(num_chains, 0.0);
    
    numAttemptedSwaps = std::vector< std::vector<unsigned long> > (num_chains, std::vector<unsigned long> (num_chains, 0));
    numAcceptedSwaps = std::vector< std::vector<unsigned long> > (num_chains, std::vector<unsigned long> (num_chains, 0));
    
    chain_moves_tuningInfo = std::vector< std::vector<Mcmc::tuningInfo> > (num_chains);
    
    if (sm == "neighbor")
    {
        useNeighborSwapping = true;
        useRandomSwapping = false;
    }
    else if (sm == "random")
    {
        useNeighborSwapping = false;
        useRandomSwapping = true;
    }
    else if (sm == "both")
    {
        useNeighborSwapping = true;
        useRandomSwapping = true;
    }

    
    // assign chains to processors, instantiate Mcmc objects
    base_chain = new Mcmc(m, mv, mn);
    
    
    // initialize the individual chains
    initializeChains();
}


Mcmcmc::Mcmcmc(const Mcmcmc &m) : MonteCarloSampler(m)
{
    
    delta               = m.delta;
    num_chains          = m.num_chains;
    heat_ranks          = m.heat_ranks;
    heat_temps          = m.heat_temps;
    swap_interval       = m.swap_interval;
    swap_interval2      = m.swap_interval2;
    tune_heat           = m.tune_heat;
    useNeighborSwapping = m.useNeighborSwapping;
    useRandomSwapping   = m.useRandomSwapping;
    
    active_chain_index  = m.active_chain_index;
    schedule_type       = m.schedule_type;
    pid_per_chain       = m.pid_per_chain;
    
    numAttemptedSwaps   = m.numAttemptedSwaps;
    numAcceptedSwaps    = m.numAcceptedSwaps;
    generation          = m.generation;
    
    
    chains.clear();
    chains.resize(num_chains, NULL);
    for (size_t i = 0; i < num_chains; ++i)
    {
        if ( m.chains[i] != NULL)
        {
            chains[i]   = m.chains[i]->clone();
        }
        
    }
    
    chain_values         = m.chain_values;
    chain_heats          = m.chain_heats;
    chain_moves_tuningInfo = m.chain_moves_tuningInfo;
    
    burnin_generation   = m.burnin_generation;
    current_generation   = m.current_generation;
    base_chain           = m.base_chain->clone();
    
}

Mcmcmc::~Mcmcmc(void)
{
    for (size_t i = 0; i < chains.size(); ++i)
    {
        if (chains[i] != NULL)
        {
            delete chains[i];
        }
    }
    chains.clear();
    delete base_chain;
}


void Mcmcmc::addFileMonitorExtension(const std::string &s, bool dir)
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            chains[i]->addFileMonitorExtension(s, dir);
        }
    }
    
}


void Mcmcmc::addMonitor(const Monitor &m)
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            chains[i]->addMonitor( m );
        }
    }
    
}


double Mcmcmc::computeBeta(double d, size_t idx)
{
    
    return 1.0 / (1.0+delta*idx);
}


Mcmcmc* Mcmcmc::clone(void) const
{
    return new Mcmcmc(*this);
}


void Mcmcmc::disableScreenMonitor( bool all, size_t rep )
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->disableScreenMonitor(all, rep);
        }
        
    }
    
}


/**
 * Start the monitors at the beginning of a run which will simply delegate this call to each chain.
 */
void Mcmcmc::finishMonitors( size_t n_reps )
{
    
    // Monitor
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->finishMonitors( n_reps );
        }
    }
    
}



/**
 * Get the model instance.
 */
const Model& Mcmcmc::getModel( void ) const
{
    
    return chains[0]->getModel();
}


double Mcmcmc::getModelLnProbability( bool likelihood_only )
{
    // we need to make sure that the vector chain_values is propagated properly
    // usualy we store the posteriors in there
    // now we might want to get the likelihoods only
    synchronizeValues(likelihood_only);
    
    // create the return value
    double rv = RbConstants::Double::neginf;
    
    for (size_t i=0; i<num_chains; ++i)
    {
        if ( chain_heats[i] == 1.0 )
        {
            rv = chain_values[i];
            break;
        }
    }
    
    // we need to make sure that the vector chain_values is propagated properly
    // usualy we store the posteriors in there
    // now we might want to get the likelihoods only
    synchronizeValues(false);
    
    return rv;
}


RbVector<Monitor>& Mcmcmc::getMonitors( void )
{
    RbVector<Monitor> *monitors = new RbVector<Monitor>();
    for (size_t i = 0; i < num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            RbVector<Monitor>& m = chains[i]->getMonitors();
            for (size_t j = 0; j < m.size(); ++j)
            {
                monitors->push_back( m[j] );
            }
        }
    }
    return *monitors;
}


std::string Mcmcmc::getStrategyDescription( void ) const
{
    std::string description = "";
    std::stringstream stream;
    stream << "The MCMCMC simulator runs 1 cold chain and " << (num_chains-1) << " heated chains.\n";
    //    stream << chains[ chainsPerProcess[pid][0] ]->getStrategyDescription();
    size_t chain_index = 0;
    while ( chain_index < num_chains && chains[chain_index] == NULL ) ++chain_index;
    
    stream << chains[chain_index]->getStrategyDescription();
    description = stream.str();
    
    return description;
}


void Mcmcmc::initializeChains(void)
{
    
    double processors_per_chain = double(num_processes) / double(num_chains);
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        // all chains know heat-order and chain-processor schedules
        heat_ranks[i] = i;
        
        // get chain heat
        if (heat_temps[0] != 0.0)
        {
            chain_heats[i] = heat_temps[i];
        }
        else
        {
            double b = computeBeta(delta,i);
            chain_heats[i] = b;
        }
        
        chain_moves_tuningInfo[i] = base_chain->getMovesTuningInfo();
        
        size_t active_pid_for_chain     = size_t( floor( i     * processors_per_chain ) + active_PID);
        size_t num_processer_for_chain  = size_t( floor( (i+1) * processors_per_chain ) + active_PID) - active_pid_for_chain;
        if ( num_processer_for_chain < 1 )
        {
            num_processer_for_chain = 1;
        }
        pid_per_chain[i] = active_pid_for_chain;
        

        // add chain to pid's chain vector (smaller memory footprint)
        if ( pid >= active_pid_for_chain && pid < (active_pid_for_chain + num_processer_for_chain) )
        {

            // create chains
            Mcmc* oneChain = new Mcmc( *base_chain );
            oneChain->setScheduleType( schedule_type );
            oneChain->setChainActive( i == 0 );
            oneChain->setChainPosteriorHeat( chain_heats[i] );
            oneChain->setChainIndex( i );
            oneChain->setActivePID( active_pid_for_chain, num_processer_for_chain );
            chains[i] = oneChain;
        }
        else
        {
            chains[i] = NULL;
        }
    }
    
}


void Mcmcmc::initializeSampler( bool priorOnly )
{
    
    // initialize each chain
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->initializeSampler( priorOnly );
        }
    }
    
}


void Mcmcmc::monitor(unsigned long g)
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL && chains[i]->isChainActive() )
        {
            chains[i]->monitor(g);
        }
        
    }
    
}

void Mcmcmc::nextCycle(bool advanceCycle)
{
    
    // run each chain for this process
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            // advance chain j by a single cycle
            chains[i]->nextCycle( advanceCycle );
        }
        
    } // loop over chains for this process
    
    if ( advanceCycle == true )
    {
        // advance gen counter
        ++current_generation;
    }
    else
    {
        ++burnin_generation;
    }
    
    if (useNeighborSwapping == true && useRandomSwapping == true)
    {
        if ((current_generation == 0 && burnin_generation % swap_interval == 0) || (current_generation > 0 && current_generation % swap_interval == 0))
        {
            
#ifdef RB_MPI
            // wait until all chains complete
            //        MPI::COMM_WORLD.Barrier();
#endif
            
            // perform chain swap
            for (size_t i = 0; i < num_chains; ++i)
            {
                swapChains("neighbor");
            }
        }
        if ((current_generation == 0 && burnin_generation % swap_interval2 == 0) || (current_generation > 0 && current_generation % swap_interval2 == 0))
        {
            
#ifdef RB_MPI
            // wait until all chains complete
            //        MPI::COMM_WORLD.Barrier();
#endif
            
            // perform chain swap
            for (size_t i = 0; i < num_chains; ++i)
            {
                for (size_t j = 0; j < num_chains; ++j)
                {
                    swapChains("random");
                }
            }
        }
    }
    else if (useNeighborSwapping == true)
    {
        if ((current_generation == 0 && burnin_generation % swap_interval == 0) || (current_generation > 0 && current_generation % swap_interval == 0))
        {
            
#ifdef RB_MPI
            // wait until all chains complete
            //        MPI::COMM_WORLD.Barrier();
#endif
            
            // perform chain swap
            for (size_t i = 0; i < num_chains; ++i)
            {
                swapChains("neighbor");
            }
        }
    }
    else if (useRandomSwapping == true)
    {
        if ((current_generation == 0 && burnin_generation % swap_interval == 0) || (current_generation > 0 && current_generation % swap_interval == 0))
        {
            
#ifdef RB_MPI
            // wait until all chains complete
            //        MPI::COMM_WORLD.Barrier();
#endif
            
            // perform chain swap
            for (size_t i = 0; i < num_chains; ++i)
            {
                for (size_t j = 0; j < num_chains; ++j)
                {
                    swapChains("random");
                }
            }
        }
    }
    
}

void Mcmcmc::printOperatorSummary(void) const
{

#ifdef RB_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        
#ifdef RB_MPI
        MPI_Barrier(MPI_COMM_WORLD);
#endif
        size_t chainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), i) - heat_ranks.begin();
        
        if (pid == pid_per_chain[chainIdx])
        {
            if (chains[chainIdx] != NULL)
            {
                chains[chainIdx]->printOperatorSummary();
            }
        }
        
#ifdef RB_MPI
        MPI_Barrier(MPI_COMM_WORLD);
#endif
    }

    
    if (num_chains > 1 && active_PID == pid)
    {
        printSummary(std::cout);
        std::cout << std::endl;
    }
    
}


/**
 * Print the summary of the move.
 *
 * The summary just contains the current value of the tuning parameter.
 * It is printed to the stream that it passed in.
 *
 * \param[in]     o     The stream to which we print the summary.
 */
void Mcmcmc::printSummary(std::ostream &o) const
{
    
    std::streamsize previousPrecision = o.precision();
    std::ios_base::fmtflags previousFlags = o.flags();
    
    o << std::fixed;
    o << std::setprecision(4);
    
    std::cout << std::endl;
    
    if (useNeighborSwapping == true && useRandomSwapping == true)
    {
        std::cout << "MCMCMC chains swapping between|swapIntervalNeighbor|swapIntervalRandom| Tried | Accepted | Acc. Ratio |  HeatFrom  |  HeatTo   " << std::endl;
        std::cout << "===============================================================================================================================" << std::endl;
        
        for (size_t i = 0; i < num_chains - 1; ++i)
        {
            for (size_t j = i + 1; j < num_chains; ++j)
            {
                printSummaryPair(o, i, j);
                printSummaryPair(o, j, i);
            }
        }

    }
    else
    {
        std::cout << "MCMCMC chains swapping between|              swapInterval             | Tried | Accepted | Acc. Ratio |  HeatFrom  |  HeatTo   " << std::endl;
        std::cout << "===============================================================================================================================" << std::endl;
        
        if (useRandomSwapping == true)
        {
            for (size_t i = 0; i < num_chains - 1; ++i)
            {
                for (size_t j = i + 1; j < num_chains; ++j)
                {
                    printSummaryPair(o, i, j);
                    printSummaryPair(o, j, i);
                }
            }
        }
        else if (useNeighborSwapping == true)
        {
            for (size_t i = 0; i < num_chains - 1; ++i)
            {
                printSummaryPair(o, i, i + 1);
            }
            
            for (size_t i = 1; i < num_chains; ++i)
            {
                printSummaryPair(o, i, i - 1);
            }
        }

    }
    
    o.setf(previousFlags);
    o.precision(previousPrecision);

}


void Mcmcmc::printSummaryPair(std::ostream &o, const size_t &row, const size_t &col) const
{
    // print the name
    o << row + 1;
    const std::string n1 = " to ";
    o << n1;
    o << col + 1;
    
    size_t n_length = n1.length() + (size_t)log10(row + 1) + (size_t)log10(col + 1);
    size_t spaces = 30 - (n_length > 30 ? 30 : n_length);
    for (size_t i = 0; i < spaces; ++i)
    {
        o << " ";
    }
    o << " ";
    
    // print the swap interval
    if (swap_interval2 > 0)
    {
        int si_length = 18;
        if (swap_interval > 0) si_length -= (int)log10(swap_interval);
        for (int i = 0; i < si_length; ++i)
        {
            o << " ";
        }
        o << swap_interval;
        o << " ";
        
        int si2_length = 16 - (int)log10(swap_interval2);
        for (int i = 0; i < si2_length; ++i)
        {
            o << " ";
        }
        o << swap_interval2;
        o << " ";
    }
    else if (swap_interval2 == 0)
    {
        int si_length = 36;
        if (swap_interval > 0) si_length -= (int)log10(swap_interval);
        for (int i = 0; i < si_length; ++i)
        {
            o << " ";
        }
        o << swap_interval;
        o << " ";
    }
    
    // print the number of tries
    int t_length = 6;
    if (numAttemptedSwaps[row][col] > 0) t_length -= (int)log10(numAttemptedSwaps[row][col]);
    for (int i = 0; i < t_length; ++i)
    {
        o << " ";
    }
    o << numAttemptedSwaps[row][col];
    o << " ";
    
    // print the number of accepted
    int a_length = 9;
    if (numAcceptedSwaps[row][col] > 0) a_length -= (int)log10(numAcceptedSwaps[row][col]);
    for (int i = 0; i < a_length; ++i)
    {
        o << " ";
    }
    o << numAcceptedSwaps[row][col];
    o << " ";
    
    // print the acceptance ratio
    double ratio = numAcceptedSwaps[row][col] / (double)numAttemptedSwaps[row][col];
    if (numAttemptedSwaps[row][col] == 0) ratio = 0;
    
    int r_length = 6;
    for (int i = 0; i < r_length; ++i)
    {
        o << " ";
    }
    o << ratio;
    o << " ";
    
    size_t rowChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), row) - heat_ranks.begin();
    size_t colChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), col) - heat_ranks.begin();
    
    // print the heat of the chain that swaps are proposed from
    int h_length = 6;
    for (int i = 0; i < h_length; ++i)
    {
        o << " ";
    }
    o << chain_heats[rowChainIdx];
    o << " ";
    
    // print the heat of the chain that swaps are proposed to
    h_length = 5;
    for (int i = 0; i < h_length; ++i)
    {
        o << " ";
    }
    o << chain_heats[colChainIdx];
    o << " ";
    
    o << std::endl;
    
}


void Mcmcmc::redrawStartingValues( void )
{
    
    // initialize each chain
    for (size_t i = 0; i < num_chains; ++i)
    {
        RandomNumberGenerator *rng = GLOBAL_RNG;
        for (size_t j=0; j<10; ++j) rng->uniform01();
        
        if ( chains[i] != NULL )
        {
            chains[i]->redrawStartingValues();
        }
    }
    
}


void Mcmcmc::removeMonitors( void )
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->removeMonitors();
        }
        
    }
    
}


void Mcmcmc::reset( void )
{
    
    // reset counters
    resetCounters();
    
    //    /* Reset the monitors */
    //    for (size_t i = 0; i < chainsPerProcess[pid].size(); ++i)
    //    {
    //        RbVector<Monitor>& monitors = chains[ chainsPerProcess[pid][i] ]->getMonitors();
    //        for (size_t i=0; i<monitors.size(); ++i)
    //        {
    //            monitors[i].reset();
    //        }
    //    }
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->reset();
        }
        
    }
    
    
}


void Mcmcmc::resetCounters( void )
{
    for (size_t i = 0; i < num_chains; ++i)
    {
        for (size_t j = 0; j < num_chains; ++j)
        {
            numAttemptedSwaps[i][j] = 0;
            numAcceptedSwaps[i][j] = 0;
//            std::cout << "numAttemptedSwaps[" << i << "][" << j << "]=" << numAttemptedSwaps[i][j] << ", numAcceptedSwaps[" << i << "][" << j << "]=" << numAcceptedSwaps[i][j] << "; ";
        }
    }
//    std::cout << std::endl;
}


void Mcmcmc::setHeatsInitial(const std::vector<double> &ht)
{
    heat_temps = ht;
}


void Mcmcmc::setSwapInterval2(const size_t &si2)
{
    swap_interval2 = si2;
}


/**
 * Set the heat of the likelihood of the current chain.
 * This heat is used in posterior posterior MCMC algorithms to
 * heat the likelihood
 * The heat is passed to the moves for the accept-reject mechanism.
 */
void Mcmcmc::setLikelihoodHeat(double h)
{
    
    for (size_t i = 0; i < num_chains; ++i)
    {
        if (chains[i] != NULL)
        {
            chains[i]->setLikelihoodHeat( h );
        }
        
    }
    
}


/**
  * Set the model by delegating the model to the chains.
  */
void Mcmcmc::setModel( Model *m, bool redraw )
{
    
    // set the models of the chains
    for (size_t i = 0; i < num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            Model *m_clone = m->clone();
            chains[i]->setModel( m_clone, redraw );
        }
        
    }
    
    if ( base_chain != NULL )
    {
        Model *m_clone = m->clone();
        base_chain->setModel( m_clone, redraw );
    }
    
    delete m;
    
}

void Mcmcmc::setActivePIDSpecialized(size_t i, size_t n)
{
    
    // initialize container sizes
    for (size_t i = 0; i < chains.size(); ++i)
    {
        
        if (chains[i] != NULL)
        {
            delete chains[i];
        }
        
    }
    
    chains.clear();
    chain_values.clear();
    chain_heats.clear();
    heat_ranks.clear();
    chain_moves_tuningInfo.clear();
    
    chains.resize(num_chains);
    chain_values.resize(num_chains, 0.0);
    chain_heats.resize(num_chains, 0.0);
    pid_per_chain.resize(num_chains, 0);
    heat_ranks.resize(num_chains, 0);
    
    chain_moves_tuningInfo = std::vector< std::vector<Mcmc::tuningInfo> > (num_chains);
    
    initializeChains();
}


/**
 * Start the monitors at the beginning of a run which will simply delegate this call to each chain.
 */
void Mcmcmc::startMonitors(size_t num_cycles, bool reopen)
{
    
    // Monitor
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->startMonitors( num_cycles, reopen );
        }
        
    }
    
}


void Mcmcmc::synchronizeValues( bool likelihood_only )
{
    
    // synchronize chain values
    double results[num_chains];
    for (size_t j = 0; j < num_chains; ++j)
    {
        results[j] = 0.0;
    }
    for (size_t j = 0; j < num_chains; ++j)
    {
        
        if ( chains[j] != NULL )
        {
            results[j] = chains[j]->getModelLnProbability(likelihood_only);
//            std::cout << "results[" << j << "]=" << results[j] << ", ";
        }
        
    }
//    std::cout << std::endl;
    
#ifdef RB_MPI
    if ( active_PID != pid )
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            if ( pid == pid_per_chain[i] )
            {
                MPI_Send(&results[i], 1, MPI_DOUBLE, (int)active_PID, 0, MPI_COMM_WORLD);
            }
            
        }
        
    }
#endif
    
    if ( active_PID == pid )
    {
#ifdef RB_MPI
        
        for (size_t j = 0; j < num_chains; ++j)
        {
            
            // ignore self
            if (pid != pid_per_chain[j])
            {
                MPI_Status status;
                MPI_Recv(&results[j], 1, MPI_DOUBLE, int(pid_per_chain[j]), 0, MPI_COMM_WORLD, &status);
            }
            
        }
#endif
        for (size_t i = 0; i < num_chains; ++i)
        {
            chain_values[i] = results[i];
        }
    }
    
#ifdef RB_MPI
    if ( active_PID == pid )
    {
        for (size_t i=1; i<num_processes; ++i)
        {
            for (size_t j=0; j<num_chains; ++j)
            {
                MPI_Send(&chain_values[j], 1, MPI_DOUBLE, int(active_PID+i), 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            MPI_Status status;
            MPI_Recv(&chain_values[i], 1, MPI_DOUBLE, int(active_PID), 0, MPI_COMM_WORLD, &status);
        }
        
    }
#endif
    
}

void Mcmcmc::synchronizeHeats(void)
{
    
    // synchronize heat values
    double heats[num_chains];
    for (size_t j = 0; j < num_chains; ++j)
    {
        heats[j] = 0.0;
    }
    for (size_t j = 0; j < num_chains; ++j)
    {
        if (chains[j] != NULL)
        {
            heats[j] = chains[j]->getChainPosteriorHeat();
        }
    }
    
#ifdef RB_MPI
    // share the heats accross processes
    if ( active_PID != pid )
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            if ( pid == pid_per_chain[i] )
            {
                MPI_Send(&heats[i], 1, MPI_DOUBLE, int(active_PID), 0, MPI_COMM_WORLD);
            }
            
        }
        
    }
#endif
    
    if ( active_PID == pid )
    {
#ifdef RB_MPI
        for (size_t j = 0; j < num_chains; ++j)
        {
            
            // ignore self
            if (pid != pid_per_chain[j])
            {
                MPI_Status status;
                MPI_Recv(&heats[j], 1, MPI_DOUBLE, int(pid_per_chain[j]), 0, MPI_COMM_WORLD, &status);
            }
            
        }
        
#endif
        for (size_t i = 0; i < num_chains; ++i)
        {
            chain_heats[i] = heats[i];
        }
    }
#ifdef RB_MPI
    if ( active_PID == pid )
    {
        for (size_t i=1; i<num_processes; ++i)
        {
            
            for (size_t j = 0; j < num_chains; ++j)
            {
                MPI_Send(&chain_heats[j], 1, MPI_DOUBLE, int(active_PID+i), 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            MPI_Status status;
            MPI_Recv(&chain_heats[i], 1, MPI_DOUBLE, int(active_PID), 0, MPI_COMM_WORLD, &status);
        }
        
    }
#endif
    
}


void Mcmcmc::synchronizeTuningInfo(void)
{
    
    // synchronize tuning information
    std::vector< std::vector<Mcmc::tuningInfo> > chain_mvs_ti = std::vector< std::vector<Mcmc::tuningInfo> > (num_chains);
    for (size_t j = 0; j < num_chains; ++j)
    {
        if (chains[j] != NULL)
        {
            chain_mvs_ti[j] = chains[j]->getMovesTuningInfo();
        }
        else
        {
            chain_mvs_ti[j] = base_chain->getMovesTuningInfo();
        }
    }
    
#ifdef RB_MPI
    // create MPI type
    const int num_items = 3;
    int block_lengths[num_items] = {1, 1, 1};
    MPI_Datatype types[num_items] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, MPI_DOUBLE};
    MPI_Datatype tmp_mvs_ti_types, mvs_ti_types;
    MPI_Aint offsets[num_items];
    
    offsets[0] = offsetof(Mcmc::tuningInfo, num_tried);
    offsets[1] = offsetof(Mcmc::tuningInfo, num_accepted);
    offsets[2] = offsetof(Mcmc::tuningInfo, tuning_parameter);
    
    MPI_Type_create_struct(num_items, block_lengths, offsets, types, &tmp_mvs_ti_types);
    
    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_mvs_ti_types, &lb, &extent);
    
    MPI_Type_create_resized(tmp_mvs_ti_types, lb, extent, &mvs_ti_types);
    MPI_Type_commit(&mvs_ti_types);
    
    // share the heats accross processes
    if ( active_PID != pid )
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            if ( pid == pid_per_chain[i] )
            {
                MPI_Send(&chain_mvs_ti[i].front(), chain_mvs_ti[i].size(), mvs_ti_types, int(active_PID), 0, MPI_COMM_WORLD);
            }
            
        }
        
    }
#endif
    
    if ( active_PID == pid )
    {
#ifdef RB_MPI
        for (size_t j = 0; j < num_chains; ++j)
        {
            
            // ignore self
            if (pid != pid_per_chain[j])
            {
                MPI_Status status;
                MPI_Recv(&chain_mvs_ti[j].front(), chain_mvs_ti[j].size(), mvs_ti_types, int(pid_per_chain[j]), 0, MPI_COMM_WORLD, &status);
            }
            
        }
        
#endif
        for (size_t i = 0; i < num_chains; ++i)
        {
            chain_moves_tuningInfo[i] = chain_mvs_ti[i];
        }
    }
    
#ifdef RB_MPI
    if ( active_PID == pid )
    {
        for (size_t i=1; i<num_processes; ++i)
        {
            
            for (size_t j = 0; j < num_chains; ++j)
            {
                MPI_Send(&chain_moves_tuningInfo[j].front(), chain_moves_tuningInfo[j].size(), mvs_ti_types, int(active_PID+i), 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for (size_t i=0; i<num_chains; ++i)
        {
            MPI_Status status;
            MPI_Recv(&chain_moves_tuningInfo[i].front(), chain_moves_tuningInfo[i].size(), mvs_ti_types, int(active_PID), 0, MPI_COMM_WORLD, &status);
        }
        
    }
    
    // free the datatype
    MPI_Type_free(&mvs_ti_types);
    
#endif
    
}


// MJL: allow swapChains to take a swap function -- e.g. pairwise swap for 1..n-1
void Mcmcmc::swapChains(const std::string swap_method)
{
    
    // exit if there is only one chain
    if (num_chains < 2)
    {
        return;
    }
    
    // send all chain values to pid 0
    synchronizeValues( false );
    
    // send all chain heats to pid 0
    synchronizeHeats();
    
    // send all chain moves tuning information to pid 0
    synchronizeTuningInfo();
    
    // swap chains
    if (swap_method == "neighbor")
    {
        swapNeighborChains();
    }
    else if (swap_method == "random")
    {
        swapRandomChains();
    }

}


void Mcmcmc::swapMovesTuningInfo(RbVector<Move> &mvsj, RbVector<Move> &mvsk)
{
    
    RbIterator<Move> itk = mvsk.begin();
    for (RbIterator<Move> itj = mvsj.begin(); itj != mvsj.end(); ++itj, ++itk)
    {
        if (itj->getMoveName() != itk->getMoveName())
        {
            throw RbException( "The two moves whose tuning information is attempted to be swapped are not the same move as their names do not match." );
        }
        
        size_t tmp_numTriedj = itj->getNumberTried();
        size_t tmp_numTriedk = itk->getNumberTried();
        itj->setNumberTried(tmp_numTriedk);
        itk->setNumberTried(tmp_numTriedj);
        
        size_t tmp_numAcceptedj = itj->getNumberAccepted();
        size_t tmp_numAcceptedk = itk->getNumberAccepted();
        itj->setNumberAccepted(tmp_numAcceptedk);
        itk->setNumberAccepted(tmp_numAcceptedj);
        
        double tmp_tuningParameterj = itj->getMoveTuningParameter();
        double tmp_tuningParameterk = itk->getMoveTuningParameter();
        
        if ((std::isnan(tmp_tuningParameterj) == true && std::isnan(tmp_tuningParameterk) == false) || (std::isnan(tmp_tuningParameterj) == false && std::isnan(tmp_tuningParameterk) == true))
        {
            throw RbException( "The two moves whose tuning information is attempted to be swapped are not the same move as only one of them has tuning parameter." );
        }
        else if (std::isnan(tmp_tuningParameterj) == false)
        {
            itj->setMoveTuningParameter(tmp_tuningParameterk);
            itk->setMoveTuningParameter(tmp_tuningParameterj);
        }
    }
    
    if (itk != mvsk.end())
    {
        throw RbException( "The two moves objetcs whose tuning information is attempted to be swapped have different number of moves." );
    }
    
}


void Mcmcmc::swapNeighborChains(void)
{
    
    double lnProposalRatio = 0.0;
    
    // randomly pick the indices of two chains
    int j = 0;
    int k = 0;
    
    // swap?
    bool accept = false;
    
    j = int(GLOBAL_RNG->uniform01() * (num_chains-1));
    k = j + 1;
    
    ++numAttemptedSwaps[heat_ranks[j]][heat_ranks[k]];
    
    // compute exchange ratio
    double bj = chain_heats[j];
    double bk = chain_heats[k];
    double lnPj = chain_values[j];
    double lnPk = chain_values[k];
    double lnR = bj * (lnPk - lnPj) + bk * (lnPj - lnPk) + lnProposalRatio;
//    std::cout << "bj=" << bj << ", bk=" << bk << ", lnPj=" << lnPj << ", lnPk=" << lnPk << ", lnR=" << lnR << std::endl;
    
    // determine whether we accept or reject the chain swap
    double u = GLOBAL_RNG->uniform01();
    if (lnR >= 0)
    {
        accept = true;
    }
    else if (lnR < -100)
    {
        accept = false;
    }
    else if (u < exp(lnR))
    {
        accept = true;
    }
    else
    {
        accept = false;
    }
    
    
#ifdef RB_MPI
    if ( active_PID == pid )
    {
        for (size_t i = 1; i < num_processes; ++i)
        {
            MPI_Send(&j, 1, MPI_INT, int(active_PID+i), 0, MPI_COMM_WORLD);
            MPI_Send(&k, 1, MPI_INT, int(active_PID+i), 0, MPI_COMM_WORLD);
            MPI_Send(&accept, 1, MPI_C_BOOL, int(active_PID+i), 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Status status;
        MPI_Recv(&j, 1, MPI_INT, int(active_PID), 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&k, 1, MPI_INT, int(active_PID), 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&accept, 1, MPI_C_BOOL, int(active_PID), 0, MPI_COMM_WORLD, &status);
    }
#endif
    
    // on accept, swap beta values and active chains
    if (accept == true )
    {
        ++numAcceptedSwaps[heat_ranks[j]][heat_ranks[k]];
        
        // swap active chain
        if (active_chain_index == j)
        {
            active_chain_index = k;
        }
        else if (active_chain_index == k)
        {
            active_chain_index = j;
        }
        
        double bj = chain_heats[j];
        double bk = chain_heats[k];
        chain_heats[j] = bk;
        chain_heats[k] = bj;
        size_t tmp = heat_ranks[j];
        heat_ranks[j] = heat_ranks[k];
        heat_ranks[k] = tmp;
        
        // also swap the tuning information of each move
//        RbVector<Move>& tmp_movesj = chains[j]->getMoves();
//        RbVector<Move>& tmp_movesk = chains[k]->getMoves();
//        swapMovesTuningInfo(tmp_movesj, tmp_movesk);
//        chains[j]->setMoves(tmp_movesj);
//        chains[k]->setMoves(tmp_movesk);
        
        std::vector<Mcmc::tuningInfo> tmp_mvs_ti = chain_moves_tuningInfo[j];
        chain_moves_tuningInfo[j] = chain_moves_tuningInfo[k];
        chain_moves_tuningInfo[k] = tmp_mvs_ti;

        
        for (size_t i=0; i<num_chains; ++i)
        {
            
            if ( chains[i] != NULL )
            {
                chains[i]->setChainPosteriorHeat( chain_heats[i] );
                chains[i]->setMovesTuningInfo(chain_moves_tuningInfo[i]);
                chains[i]->setChainActive( chain_heats[i] == 1.0 );
            }
        }
        
    }
    
    
    // update the chains accross processes
    // this is necessary because only process 0 does the swap
    // all the other processes need to be told that there was a swap
    //    updateChainState(j);
    //    updateChainState(k);
    
}



void Mcmcmc::swapRandomChains(void)
{
    
    double lnProposalRatio = 0.0;
    
    // randomly pick the indices of two chains
    int j = 0;
    int k = 0;
    
    // swap?
    bool accept = false;
    
    j = int(GLOBAL_RNG->uniform01() * num_chains);
    if (num_chains > 1)
    {
        do {
            k = int(GLOBAL_RNG->uniform01() * num_chains);
        }
        while(j == k);
    }
    
    ++numAttemptedSwaps[heat_ranks[j]][heat_ranks[k]];
    
    // compute exchange ratio
    double bj = chain_heats[j];
    double bk = chain_heats[k];
    double lnPj = chain_values[j];
    double lnPk = chain_values[k];
    double lnR = bj * (lnPk - lnPj) + bk * (lnPj - lnPk) + lnProposalRatio;
    //        std::cout << "bj=" << bj << ", bk=" << bk << ", lnPj=" << lnPj << ", lnPk=" << lnPk << ", lnR=" << lnR << std::endl;
    
    // determine whether we accept or reject the chain swap
    double u = GLOBAL_RNG->uniform01();
    if (lnR >= 0)
    {
        accept = true;
    }
    else if (lnR < -100)
    {
        accept = false;
    }
    else if (u < exp(lnR))
    {
        accept = true;
    }
    else
    {
        accept = false;
    }
    
    
#ifdef RB_MPI
    if ( active_PID == pid )
    {
        for (size_t i = 1; i < num_processes; ++i)
        {
            MPI_Send(&j, 1, MPI_INT, int(active_PID+i), 0, MPI_COMM_WORLD);
            MPI_Send(&k, 1, MPI_INT, int(active_PID+i), 0, MPI_COMM_WORLD);
            MPI_Send(&accept, 1, MPI_C_BOOL, int(active_PID+i), 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Status status;
        MPI_Recv(&j, 1, MPI_INT, int(active_PID), 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&k, 1, MPI_INT, int(active_PID), 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&accept, 1, MPI_C_BOOL, int(active_PID), 0, MPI_COMM_WORLD, &status);
    }
#endif
    
    
    // on accept, swap beta values and active chains
    if (accept == true )
    {
        ++numAcceptedSwaps[heat_ranks[j]][heat_ranks[k]];
        
        // swap active chain
        if (active_chain_index == j)
        {
            active_chain_index = k;
        }
        else if (active_chain_index == k)
        {
            active_chain_index = j;
        }
        
        double bj = chain_heats[j];
        double bk = chain_heats[k];
        chain_heats[j] = bk;
        chain_heats[k] = bj;
        size_t tmp = heat_ranks[j];
        heat_ranks[j] = heat_ranks[k];
        heat_ranks[k] = tmp;
        
        // also swap the tuning information of each move
        //            RbVector<Move>& tmp_movesj = chains[j]->getMoves();
        //            RbVector<Move>& tmp_movesk = chains[k]->getMoves();
        //            swapMovesTuningInfo(tmp_movesj, tmp_movesk);
        //            chains[j]->setMoves(tmp_movesj);
        //            chains[k]->setMoves(tmp_movesk);
        
        std::vector<Mcmc::tuningInfo> tmp_mvs_ti = chain_moves_tuningInfo[j];
        chain_moves_tuningInfo[j] = chain_moves_tuningInfo[k];
        chain_moves_tuningInfo[k] = tmp_mvs_ti;
        
        
        for (size_t i=0; i<num_chains; ++i)
        {
            
            if ( chains[i] != NULL )
            {
                chains[i]->setChainPosteriorHeat( chain_heats[i] );
                chains[i]->setMovesTuningInfo(chain_moves_tuningInfo[i]);
                chains[i]->setChainActive( chain_heats[i] == 1.0 );
            }
        }
        
    }
    
    
    // update the chains accross processes
    // this is necessary because only process 0 does the swap
    // all the other processes need to be told that there was a swap
    //    updateChainState(j);
    //    updateChainState(k);
    
}


void Mcmcmc::tune( void )
{
    if (tune_heat == true && num_chains > 1)
    {
        double tuneHeatTarget = 0.23;
        
        std::vector<double> heats_diff(num_chains - 1, 0.0);
        
        for (size_t i = 1; i < num_chains; ++i)
        {
            size_t colderChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), i - 1) - heat_ranks.begin();
            size_t hotterChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), i) - heat_ranks.begin();
            
            heats_diff[i - 1] = chain_heats[colderChainIdx] - chain_heats[hotterChainIdx];
        }
        
        for (size_t i = 1; i < num_chains; ++i)
        {
//            std::cout << ", numAcceptedSwaps[" << i - 1 << "][" << i << "]=" << numAcceptedSwaps[i - 1][i] << ", numAcceptedSwaps[" << i << "][" << i - 1 << "]=" << numAcceptedSwaps[i][i - 1] << std::endl;
//            std::cout << ", numAttemptedSwaps[" << i - 1 << "][" << i << "]=" << numAttemptedSwaps[i - 1][i] << ", numAttemptedSwaps[" << i << "][" << i - 1 << "]=" << numAttemptedSwaps[i][i - 1] << std::endl;
            
            size_t numAttemptedSwapsPairwise = numAttemptedSwaps[i - 1][i] + numAttemptedSwaps[i][i - 1];
            if ( numAttemptedSwapsPairwise > 2 ) {
                
                size_t numAcceptedSwapsPairwise = numAcceptedSwaps[i - 1][i] + numAcceptedSwaps[i][i - 1];
                double rate = numAcceptedSwapsPairwise / (double)numAttemptedSwapsPairwise;
                
                if ( rate > tuneHeatTarget )
                {
                    heats_diff[i - 1] *= (1.0 + (rate - tuneHeatTarget) / (1.0 - tuneHeatTarget));
                }
                else
                {
                    heats_diff[i - 1] /= (2.0 - rate / tuneHeatTarget);
                }
                
//                std::cout << "rate=" << rate << ", heats_diff[" << i - 1 << "]=" << heats_diff[i - 1] << std::endl;
            }
        }
        
        double heatMinBound = 0.01;
        size_t j = 1;
        for (; j < num_chains; ++j)
        {
            size_t colderChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), j - 1) - heat_ranks.begin();
            size_t hotterChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), j) - heat_ranks.begin();
            
            chain_heats[hotterChainIdx] = chain_heats[colderChainIdx] - heats_diff[j - 1];
            
            if (chain_heats[hotterChainIdx] < heatMinBound)
            {
                break;
            }
            
//            std::cout << "chain_heats[" << hotterChainIdx << "]=" << chain_heats[hotterChainIdx] << std::endl;
        }
        
        // if the heat of a given hot chain is smaller than the minimum bound
        // interpolate this heat and the heats of all the hotter chains
        // to fall between the lowest heat that is greater than the minimum bound and the minimum bound
        if (j < num_chains)
        {
            size_t colderChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), j - 1) - heat_ranks.begin();
            double rho = pow(chain_heats[colderChainIdx] / heatMinBound, 1.0 / (num_chains - j));
            size_t k = j;
            
            for (; k < num_chains; ++k)
            {
                size_t hotterChainIdx = std::find(heat_ranks.begin(), heat_ranks.end(), k) - heat_ranks.begin();
                chain_heats[hotterChainIdx] = chain_heats[colderChainIdx] / pow(rho, k + 1 - j);
                
//                std::cout << "chain_heats[k" << hotterChainIdx << "]=" << chain_heats[hotterChainIdx] << std::endl;
            }
        }
        
        resetCounters();
    }

    
    for (size_t i=0; i<num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            chains[i]->setChainPosteriorHeat( chain_heats[i] );
            chains[i]->setChainActive( chain_heats[i] == 1.0 );
            
            chains[i]->tune();
        }
    }
    
}


void Mcmcmc::updateChainState(size_t j)
{
    
#ifdef RB_MPI
    // update heat
    if ( active_PID == pid )
    {
        for (size_t i = 1; i < num_processes; ++i)
        {
            MPI_Send(&chain_heats[j], 1, MPI_DOUBLE, int(active_PID+i), 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Status status;
        MPI_Recv(&chain_heats[j], 1, MPI_DOUBLE, int(active_PID), 0, MPI_COMM_WORLD, &status);
    }
#endif
    
    if ( chains[j] != NULL )
    {
        chains[j]->setChainPosteriorHeat( chain_heats[j] );
    }
    
    for (size_t i=0; i<num_chains; ++i)
    {
        if ( chains[i] != NULL )
        {
            chains[i]->setChainActive( chain_heats[i] == 1.0 );
        }
    }
    
}


/**
 * Start the monitors at the beginning of a run which will simply delegate this call to each chain.
 */
void Mcmcmc::writeMonitorHeaders( void )
{
    
    // Monitor
    for (size_t i = 0; i < num_chains; ++i)
    {
        
        if ( chains[i] != NULL )
        {
            chains[i]->writeMonitorHeaders();
        }
        
    }
    
}

