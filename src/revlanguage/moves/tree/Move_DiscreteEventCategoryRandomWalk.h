#ifndef Move_DiscreteEventCategoryRandomWalk_H
#define Move_DiscreteEventCategoryRandomWalk_H

#include "RlMove.h"
#include "TypedDagNode.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    
    /**
     * @brief Rev wrapper class for the BirthDeathEvent move.
     *
     * This class is the Rev wrapper class for the BirthDeathEvent move,
     * a move that proposes a new topology based on a subtree-prune
     * and regraft move with fixed node ages.
     *
     *
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @copyright GPL version 3
     */
    class Move_DiscreteEventCategoryRandomWalk : public Move {
        
    public:
        
        Move_DiscreteEventCategoryRandomWalk(void);                                                                                                                    //!< Default constructor
        
        // Basic utility functions
        virtual Move_DiscreteEventCategoryRandomWalk*       clone(void) const;                                                                      //!< Clone object
        void                                                constructInternalObject(void);                                                          //!< We construct the a new internal Move.
        static const std::string&                           getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                              getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::string                                         getMoveName(void) const;                                                                //!< Get the name used for the constructor function in Rev.
        const MemberRules&                                  getParameterRules(void) const;                                                          //!< Get member rules (const)
        virtual const TypeSpec&                             getTypeSpec(void) const;                                                                //!< Get language type of the object
        
    protected:
        
        void                                                setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
        
        RevPtr<const RevVariable>                           tree;
        
    };
    
}

#endif
