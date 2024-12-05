target_sources(ffxi PUBLIC
    FILE_SET CXX_MODULES
    FILES
    cib.cppm
    d3m.cppm
    dat.cppm
    dat_loader.cppm
    dxt3.cppm
    generator.cppm
    key_tables.cppm
    mmb.cppm
    mo2.cppm
    mzb.cppm
    os2.cppm
    sep.cppm
    scheduler.cppm
    sk2.cppm
    PRIVATE
    dat.cpp
)
