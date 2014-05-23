//
//  NodeRejectionSampleProposal.h
//  rb_mlandis
//
//  Created by Michael Landis on 5/7/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#ifndef __rb_mlandis__NodeRejectionSampleProposal__
#define __rb_mlandis__NodeRejectionSampleProposal__

#include "BranchHistory.h"
#include "DeterministicNode.h"
#include "DiscreteCharacterData.h"
#include "DistributionBinomial.h"
#include "PathRejectionSampleProposal.h"
#include "Proposal.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RateMap.h"
#include "RbException.h"
#include "StochasticNode.h"
//#include "TransitionProbability.h"
#include "TypedDagNode.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>

namespace RevBayesCore {
    
    /**
     * The scaling operator.
     *
     * A scaling proposal draws a random uniform number u ~ unif(-0.5,0.5)
     * and scales the current vale by a scaling factor
     * sf = exp( lambda * u )
     * where lambda is the tuning parameter of the Proposal to influence the size of the proposals.
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2009-09-08, version 1.0
     *
     */
    
    template<class charType, class treeType>
    class NodeRejectionSampleProposal : public Proposal {
        
    public:
        NodeRejectionSampleProposal( StochasticNode<AbstractCharacterData> *n, StochasticNode<treeType>* t, DeterministicNode<RateMap> *q, double l, int idx=-1 );                                                                //!<  constructor
        
        // Basic utility functions
        void                            assignNodeIndex(size_t idx);
        void                            assignSiteIndexSet(const std::set<size_t>& s);
        NodeRejectionSampleProposal*    clone(void) const;                                                                  //!< Clone object
        void                            cleanProposal(void);
        double                          doProposal(void);                                                                   //!< Perform proposal
        const std::vector<DagNode*>&    getNodes(void) const;                                                               //!< Get the vector of DAG nodes this proposal is working on
        const std::string&              getProposalName(void) const;                                                        //!< Get the name of the proposal for summary printing
        void                            printParameterSummary(std::ostream &o) const;                                       //!< Print the parameter summary
        void                            prepareProposal(void);                                                              //!< Prepare the proposal
        void                            sampleNodeCharacters(const TopologyNode& node, const std::set<size_t>& indexSet);   //!< Sample the characters at the node
        double                          sampleRootCharacters(const TopologyNode& node, const std::set<size_t>& indexSet);
        void                            swapNode(DagNode *oldN, DagNode *newN);                                             //!< Swap the DAG nodes on which the Proposal is working on
        void                            tune(double r);                                                                     //!< Tune the proposal to achieve a better acceptance/rejection ratio
        void                            undoProposal(void);                                                                 //!< Reject the proposal
        
    protected:
        
        // parameters
        StochasticNode<AbstractCharacterData>*  ctmc;
        StochasticNode<treeType>*               tau;
        DeterministicNode<RateMap>*             qmap;
        std::vector<DagNode*>                   nodes;

        // dimensions
        size_t                                  numNodes;
        size_t                                  numCharacters;
        size_t                                  numStates;
        
        // proposal
        std::vector<unsigned>                   storedNodeState;
        std::vector<unsigned>                   storedRootState;
        std::vector<CharacterEvent*>            storedNodeState2;
        std::vector<CharacterEvent*>            storedRootState2;
        
        std::map<size_t,BranchHistory*>         storedValues;
        std::map<size_t,BranchHistory*>         proposedValues;
        BranchHistory*                          storedValue;
        int                                     nodeIndex;
        int                                     monitorIndex;
        std::set<size_t>                        siteIndexSet;
        double                                  storedLnProb;
        BranchHistory*                          proposedValue;
        double                                  proposedLnProb;
        
        PathRejectionSampleProposal<charType,treeType>* nodeProposal;
        PathRejectionSampleProposal<charType,treeType>* leftProposal;
        PathRejectionSampleProposal<charType,treeType>* rightProposal;
        
        TransitionProbabilityMatrix nodeTpMatrix;
        TransitionProbabilityMatrix leftTpMatrix;
        TransitionProbabilityMatrix rightTpMatrix;
        
        double                                  lambda;
        
        // flags
        bool                                    fixNodeIndex;
        bool                                    sampleNodeIndex;
        bool                                    sampleSiteIndexSet;
        
    };
    
}



/**
 * Constructor
 *
 * Here we simply allocate and initialize the Proposal object.
 */
template<class charType, class treeType>
RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::NodeRejectionSampleProposal( StochasticNode<AbstractCharacterData> *n, StochasticNode<treeType> *t, DeterministicNode<RateMap>* q, double l, int idx) : Proposal(),
ctmc(n),
tau(t),
qmap(q),
numNodes(t->getValue().getNumberOfNodes()),
numCharacters(n->getValue().getNumberOfCharacters()),
numStates(static_cast<const DiscreteCharacterState&>(n->getValue().getCharacter(0,0)).getNumberOfStates()),
nodeIndex(idx),
storedValue(NULL),
proposedValue(NULL),
nodeTpMatrix(numStates),
leftTpMatrix(numStates),
rightTpMatrix(numStates),
lambda(l),
sampleNodeIndex(true),
sampleSiteIndexSet(true)

{
    monitorIndex = -120;
    nodes.push_back(ctmc);
    nodes.push_back(tau);
    nodes.push_back(qmap);

    nodeProposal = new PathRejectionSampleProposal<charType,treeType>(n,t,q,l,idx);
    leftProposal = new PathRejectionSampleProposal<charType,treeType>(n,t,q,l,idx);
    rightProposal = new PathRejectionSampleProposal<charType,treeType>(n,t,q,l,idx);
    ;
    
    fixNodeIndex = (nodeIndex > -1);
}


template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::cleanProposal( void )
{
    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
    const TopologyNode& node = tau->getValue().getNode(nodeIndex);
    const std::vector<BranchHistory*>& histories = p->getHistories();

    if (node.getIndex() == monitorIndex) { std::cout << "BEFORE clean\n"; histories[node.getIndex()]->print(); std::cout << "-----------\n"; }
    
    nodeProposal->cleanProposal();
    if (!node.isTip())
    {
        rightProposal->cleanProposal();
        leftProposal->cleanProposal();
    }
    
    if (node.getIndex() == monitorIndex) { std::cout << "AFTER clean\n"; histories[node.getIndex()]->print();
        
        std::cout << "-----------\n"; }

    
//    std::cout << "AFTER ACCEPT\n";
//    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
//    p->getHistory(node.getIndex()).print();
//    if (!node.isTip())
//    {
//        p->getHistory(node.getChild(0).getIndex()).print();
//        p->getHistory(node.getChild(1).getIndex()).print();
//    }
//    std::cout << "--------------------------\n";
}

/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * \return A new copy of the proposal.
 */
template<class charType, class treeType>
RevBayesCore::NodeRejectionSampleProposal<charType, treeType>* RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::clone( void ) const
{
    return new NodeRejectionSampleProposal( *this );
}

template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::assignNodeIndex(size_t idx)
{
    nodeIndex = idx;
    sampleNodeIndex = false;
}

template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::assignSiteIndexSet(const std::set<size_t>& s)
{
    siteIndexSet = s;
    sampleSiteIndexSet = false;
}


/**
 * Get Proposals' name of object
 *
 * \return The Proposals' name.
 */
template<class charType, class treeType>
const std::string& RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::getProposalName( void ) const
{
    static std::string name = "NodeRejectionSampleProposal";
    
    return name;
}


/**
 * Get the vector of nodes on which this proposal is working on.
 *
 * \return  Const reference to a vector of nodes pointer on which the proposal operates.
 */
template<class charType, class treeType>
const std::vector<RevBayesCore::DagNode*>& RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::getNodes( void ) const
{
    
    return nodes;
}


/**
 * Perform the Proposal.
 *
 * A scaling Proposal draws a random uniform number u ~ unif(-0.5,0.5)
 * and scales the current vale by a scaling factor
 * sf = exp( lambda * u )
 * where lambda is the tuning parameter of the Proposal to influence the size of the proposals.
 *
 * \return The hastings ratio.
 */
template<class charType, class treeType>
double RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::doProposal( void )
{
    proposedLnProb = 0.0;
    proposedValues.clear();
    
    double proposedLnProbRatio = 0.0;
    
    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
    const TopologyNode& node = tau->getValue().getNode(nodeIndex);
    const std::vector<BranchHistory*>& histories = p->getHistories();
    
    // update node value
    
    
    if (node.getIndex() == monitorIndex) { std::cout << "BEFORE node\n"; histories[node.getIndex()]->print(); }

//    sampleNodeCharacters(node, siteIndexSet);
//    if (node.isRoot())
//        proposedLnProbRatio += sampleRootCharacters(node,siteIndexSet);
//
    
    if (node.getIndex() == monitorIndex) { std::cout << "AFTER node\n"; histories[node.getIndex()]->print(); }
    
    
    
    
    // update 3x incident paths
    if (node.getIndex() == monitorIndex) { std::cout << "BEFORE path\n"; histories[node.getIndex()]->print(); }

    proposedLnProbRatio += nodeProposal->doProposal();
    if (!node.isTip())
    {
        proposedLnProbRatio += leftProposal->doProposal();
        proposedLnProbRatio += rightProposal->doProposal();
    }
    
    if (node.getIndex() == monitorIndex) { std::cout << "AFTER path\n"; histories[node.getIndex()]->print(); }

    
    return proposedLnProbRatio;
}


/**
 *
 */
template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::prepareProposal( void )
{
    
    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
        
    storedLnProb = 0.0;
    storedValues.clear();
    
    size_t numTips = tau->getValue().getNumberOfTips();
    if (sampleNodeIndex && !fixNodeIndex)
    {
        nodeIndex = numTips + GLOBAL_RNG->uniform01() * (numNodes-numTips);
    }
    
    if (sampleSiteIndexSet)
    {
        siteIndexSet.clear();
        siteIndexSet.insert(GLOBAL_RNG->uniform01() * numCharacters); // at least one is inserted
        for (size_t i = 0; i < numCharacters; i++)
        {
            if (GLOBAL_RNG->uniform01() < lambda)
            {
                siteIndexSet.insert(i);
            }
        }
    }
    
    const TopologyNode& node = tau->getValue().getNode(nodeIndex);
    
    const std::vector<BranchHistory*>& histories = p->getHistories();
    
//    std::cout << "BEFORE do\n";
//    histories[node.getIndex()]->print();
//    histories[node.getChild(0).getIndex()]->print();
//    histories[node.getChild(1).getIndex()]->print();

    nodeProposal->assignNodeIndex(node.getIndex());
    nodeProposal->assignSiteIndexSet(siteIndexSet);
    nodeProposal->prepareProposal();
    
    if (!node.isTip())
    {
        leftProposal->assignNodeIndex(node.getChild(0).getIndex());
        leftProposal->assignSiteIndexSet(siteIndexSet);
        leftProposal->prepareProposal();
        
        rightProposal->assignNodeIndex(node.getChild(1).getIndex());
        rightProposal->assignSiteIndexSet(siteIndexSet);
        rightProposal->prepareProposal();
    }
   
    // store node state values
    const std::vector<CharacterEvent*>& nodeState = p->getHistory(nodeIndex).getChildCharacters();
    for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
    {
        unsigned s = 0;
        if (nodeState[*it]->getState() == 1)
            s = 1;
        storedNodeState[*it] = s;
    }
    if (node.isRoot())
    {
        storedRootState.resize(numCharacters,0);
        const std::vector<CharacterEvent*>& nodeState = p->getHistory(nodeIndex).getParentCharacters();
        for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
        {
            unsigned s = 0;
            if (nodeState[*it]->getState() == 1)
                s = 1;
            storedRootState[*it] = s;
        }
    }
    
    // alt...
//    storedNodeState.resize(numCharacters,0);
//    storedRootState2.resize(numCharacters,0) = p->getHistory(nodeIndex).getChildCharacters();
//    storedNodeState2.resize(numCharacters,0) = p->getHistory(nodeIndex).getParentCharacters();

    
    sampleNodeIndex = true;
    sampleSiteIndexSet = true;    
}


/**
 * Print the summary of the Proposal.
 *
 * The summary just contains the current value of the tuning parameter.
 * It is printed to the stream that it passed in.
 *
 * \param[in]     o     The stream to which we print the summary.
 */
template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::printParameterSummary(std::ostream &o) const
{
    o << "lambda = " << lambda;
}


template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::sampleNodeCharacters(const TopologyNode& node, const std::set<size_t>& indexSet)
{

    if (node.isTip())
    {
        // will specify update for noisy tip data in alt. proposal
        ;
    }
    else
    {
        AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
        
        qmap->getValue().calculateTransitionProbabilities(node, nodeTpMatrix);
        qmap->getValue().calculateTransitionProbabilities(node.getChild(0), leftTpMatrix);
        qmap->getValue().calculateTransitionProbabilities(node.getChild(1), rightTpMatrix);
        
        std::vector<BranchHistory*> histories = p->getHistories();
        
        // for sampling probs
        const std::vector<CharacterEvent*>& nodeParentState = histories[node.getIndex()]->getParentCharacters();
        const std::vector<CharacterEvent*>& leftChildState = histories[node.getChild(0).getIndex() ]->getChildCharacters();
        const std::vector<CharacterEvent*>& rightChildState = histories[node.getChild(1).getIndex()]->getChildCharacters();
        
        // to update
        std::vector<CharacterEvent*> nodeChildState = histories[node.getIndex()]->getChildCharacters();
        std::vector<CharacterEvent*> leftParentState = histories[node.getChild(0).getIndex() ]->getParentCharacters();
        std::vector<CharacterEvent*> rightParentState = histories[node.getChild(1).getIndex()]->getParentCharacters();
            
        for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
        {
            unsigned int ancS = nodeParentState[*it]->getState();
            unsigned int desS1 = leftChildState[*it]->getState();
            unsigned int desS2 = rightChildState[*it]->getState();
            
            double u = GLOBAL_RNG->uniform01();
            double g0 = nodeTpMatrix[ancS][0] * leftTpMatrix[0][desS1] * rightTpMatrix[0][desS2];
            double g1 = nodeTpMatrix[ancS][1] * leftTpMatrix[1][desS1] * rightTpMatrix[1][desS2];
            
            unsigned int s = 0;
            if (u < g1 / (g0 + g1))
                s = 1;
            
            nodeChildState[*it] = new CharacterEvent(*it,s,1.0);
            leftParentState[*it] = new CharacterEvent(*it,s,0.0);
            rightParentState[*it] = new CharacterEvent(*it,s,0.0);

            
//            nodeChildState[*it]->setState(s);
//            leftParentState[*it]->setState(s);
//            rightParentState[*it]->setState(s);

            
//            std::cout << ancS << " " << desS1 << " " << desS2 << "\n";
//            std::cout << g0 << " = " << nodeTpMatrix[ancS][0] << " * " << leftTpMatrix[0][desS1] << " * " << rightTpMatrix[0][desS2] << "\n";
//            std::cout << g1 << " = " << nodeTpMatrix[ancS][1] << " * " << leftTpMatrix[1][desS1] << " * " << rightTpMatrix[1][desS2] << "\n";
//            std::cout << s << " " << g1/(g0+g1) << "\n";

            //std::cout << "---\n";
        }
        
        histories[node.getIndex()]->setChildCharacters(nodeChildState);
        histories[node.getChild(0).getIndex()]->setParentCharacters(leftParentState);
        histories[node.getChild(1).getIndex()]->setParentCharacters(rightParentState);
    }
    
    /*
     double br = branchRate->getValue();
     double rootAge = tree->getValue().getRoot().getAge();
     double bt = tree->getValue().getBranchLength(index) / rootAge;
     if (bt == 0.0) // root bt
     bt = 100.0;
     double bs = br * bt;
     //std::cout << "bs  " << bs << "\n";
     
     // compute transition probabilities
     double r[2] = { rates[0]->getValue(), rates[1]->getValue() };
     double expPart0 = exp( - (r[0] + r[1]) * bs);
     double expPart1 = exp( - (r[0] + r[1]) * t1/rootAge); // needs *br1
     double expPart2 = exp( - (r[0] + r[1]) * t2/rootAge); // needs *br2
     double pi0 = r[0] / (r[0] + r[1]);
     double pi1 = 1.0 - pi0;
     double tp0[2][2] = { { pi0 + pi1 * expPart0, pi1 - pi1 * expPart0 }, { pi0 - pi0 * expPart0, pi1 + pi0 * expPart0 } };
     double tp1[2][2] = { { pi0 + pi1 * expPart1, pi1 - pi1 * expPart1 }, { pi0 - pi0 * expPart1, pi1 + pi0 * expPart1 } };
     double tp2[2][2] = { { pi0 + pi1 * expPart2, pi1 - pi1 * expPart2 }, { pi0 - pi0 * expPart2, pi1 + pi0 * expPart2 } };
     
     //    std::cout << tp0[0][0] << " " << tp0[0][1] << "\n" << tp0[1][0] << " " << tp0[1][1] << "\n";
     //    std::cout << tp1[0][0] << " " << tp1[0][1] << "\n" << tp1[1][0] << " " << tp1[1][1] << "\n";
     //    std::cout << tp2[0][0] << " " << tp2[0][1] << "\n" << tp2[1][0] << " " << tp2[1][1] << "\n";
     
     std::vector<CharacterEvent*> parentState = value->getParentCharacters();
     std::vector<CharacterEvent*> childState = value->getChildCharacters();
     for (std::set<size_t>::iterator it = indexSet.begin(); it != indexSet.end(); it++)
     {
     unsigned int ancS = parentState[*it]->getState();
     //unsigned int thisS = childState[*it]->getState();
     unsigned int desS1 = state1[*it]->getState();
     unsigned int desS2 = state2[*it]->getState();
     
     double u = GLOBAL_RNG->uniform01();
     double g0 = tp0[ancS][0] * tp1[0][desS1] * tp2[0][desS2];
     double g1 = tp0[ancS][1] * tp1[1][desS1] * tp2[1][desS2];
     
     unsigned int s = 0;
     if (u < g1 / (g0 + g1))
     s = 1;
     
     //        std::cout << "\t" << *it << " " << s << " : " << ancS << " -> (" << g0 << "," << g1 << ") -> (" << desS1 << "," << desS2 << ")\n";
     
     childState[*it] = new CharacterEvent(*it, s, 1.0);
     }
     
     //double lnP = sampleCharacterState(indexSet, childStates, 1.0);
     value->setChildCharacters(childState);
     
     return 0.0;
     */
}

template<class charType, class treeType>
double RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::sampleRootCharacters(const TopologyNode& node, const std::set<size_t>& indexSet)
{
    double lnP = 0.0;
    
    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());

    BranchHistory* bh = &p->getHistory(node.getIndex());
    std::vector<CharacterEvent*> parentState = bh->getParentCharacters();
    
    double r0 = qmap->getValue().getSiteRate(node,1,0);
    double r1 = qmap->getValue().getSiteRate(node,0,1);
    unsigned n1_old = 0;
    unsigned n1_new = 0;
    
    double p1 = r1 / (r0 + r1);
    for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
    {
        unsigned s = 0;
        if (GLOBAL_RNG->uniform01() < p1)
        {
            s = 1;
            n1_new++;
        }
        if (parentState[*it]->getState() == 1)
        {
            n1_old++;
        }
        
        parentState[*it]->setState(s); //new CharacterEvent(*it,s,0.0);
    }
    bh->setParentCharacters(parentState);
    
    size_t n = siteIndexSet.size();
    size_t n0_old = n - n1_old;
    size_t n0_new = n - n1_new;
    double p0 = 1.0 - p1;
    
    lnP = n1_old * log(p1) + n0_old * log(p0) - n1_new * log(p1) - n0_new * log(p0);
    return 0.0;
    return lnP;
}


/**
 * Reject the Proposal.
 *
 * Since the Proposal stores the previous value and it is the only place
 * where complex undo operations are known/implement, we need to revert
 * the value of the ctmc/DAG-node to its original value.
 */
template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::undoProposal( void )
{
    AbstractTreeHistoryCtmc<charType, treeType>* p = static_cast< AbstractTreeHistoryCtmc<charType, treeType>* >(&ctmc->getDistribution());
    const TopologyNode& node = tau->getValue().getNode(nodeIndex);
    const std::vector<BranchHistory*>& histories = p->getHistories();
    
//    std::cout << "BEFORE undo\n";
//    histories[node.getIndex()]->print();
//    histories[node.getChild(0).getIndex()]->print();
//    histories[node.getChild(1).getIndex()]->print();
    
    
    if (node.getIndex() == monitorIndex) { std::cout << "BEFORE undo\n"; histories[node.getIndex()]->print(); }
    
    // restore path state
    nodeProposal->undoProposal();
    if (!node.isTip())
    {
        rightProposal->undoProposal();
        leftProposal->undoProposal();
    }
    
    // restore node state
    std::vector<CharacterEvent*> nodeChildState = histories[node.getIndex()]->getChildCharacters();
    std::vector<CharacterEvent*> leftParentState = histories[node.getChild(0).getIndex() ]->getParentCharacters();
    std::vector<CharacterEvent*> rightParentState = histories[node.getChild(1).getIndex()]->getParentCharacters();

    for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
    {
        unsigned s = storedNodeState[*it];
        nodeChildState[*it]->setState(s);
        leftParentState[*it]->setState(s);
        rightParentState[*it]->setState(s);
    }
    if (node.isRoot())
    {
        std::vector<CharacterEvent*> rootState = histories[node.getIndex()]->getParentCharacters();
        for (std::set<size_t>::iterator it = siteIndexSet.begin(); it != siteIndexSet.end(); it++)
        {
            unsigned s = storedRootState[*it];
            rootState[*it]->setState(s);
        }

    }
    
    if (node.getIndex() == monitorIndex) { std::cout << "AFTER undo\n"; histories[node.getIndex()]->print();
        
        std::cout << "-----------\n"; }
    
//    std::cout << "AFTER undo\n";
//    histories[node.getIndex()]->print();
//    histories[node.getChild(0).getIndex()]->print();
//    histories[node.getChild(1).getIndex()]->print();
//    std::cout << "\n";
    
}


/**
 * Swap the current ctmc for a new one.
 *
 * \param[in]     oldN     The old ctmc that needs to be replaced.
 * \param[in]     newN     The new ctmc.
 */
template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::swapNode(DagNode *oldN, DagNode *newN)
{
    
    if (oldN == ctmc)
    {
        ctmc = static_cast<StochasticNode<AbstractCharacterData>* >(newN) ;
    }
    else if (oldN == tau)
    {
        tau = static_cast<StochasticNode<treeType>* >(newN);
    }
    else if (oldN == qmap)
    {
        qmap = static_cast<DeterministicNode<RateMap>* >(newN);
    }
    
    nodeProposal->swapNode(oldN, newN);
    leftProposal->swapNode(oldN, newN);
    rightProposal->swapNode(oldN, newN);
}


/**
 * Tune the Proposal to accept the desired acceptance ratio. 
 */
template<class charType, class treeType>
void RevBayesCore::NodeRejectionSampleProposal<charType, treeType>::tune( double rate )
{
    ; // do nothing
}

#endif /* defined(__rb_mlandis__NodeRejectionSampleProposal__) */
