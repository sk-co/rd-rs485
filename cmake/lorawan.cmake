cmake_minimum_required(VERSION 3.16)

# Allow selection of region
option(REGION_EU868 "Region EU868" ON)
option(REGION_RU864 "Region RU864" ON)
set(REGION_LIST REGION_EU868 REGION_RU864)

# Switch for Class B support of LoRaMac.
option(CLASSB_ENABLED "Class B support of LoRaMac" ON)

# Allow switching of secure-elements
set(SECURE_ELEMENT_LIST SOFT_SE)
set(SECURE_ELEMENT SOFT_SE CACHE STRING "Default secure element is SOFT_SE")
set_property(CACHE SECURE_ELEMENT PROPERTY STRINGS ${SECURE_ELEMENT_LIST})

function(fn_add_lorawan_definitions TARGET)

  # Loops through all regions and add compile time definitions for the enabled ones.
  foreach( REGION ${REGION_LIST} )
    if(${REGION})
      target_compile_definitions(${TARGET} PUBLIC -D"${REGION}")
    endif()
  endforeach()

  # Add define if class B is supported
  target_compile_definitions(${TARGET} PRIVATE $<$<BOOL:${CLASSB_ENABLED}>:LORAMAC_CLASSB_ENABLED>)

  # SecureElement NVM
  if(${SECURE_ELEMENT} MATCHES SOFT_SE)
    target_compile_definitions(${TARGET} PRIVATE -DSOFT_SE)
  endif()
endfunction()