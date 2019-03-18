project(eos.dice)

set(HEADERS
        config.hpp
        debug_config.hpp
        logger.hpp
        tables.hpp
        eos.dice.hpp
        )

set(SOURCES ${HEADERS}
        eos.dice.cpp
        )

add_contract_project(${PROJECT_NAME} ${OUTPUT_ROOT} ${SOURCES})

