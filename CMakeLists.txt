cmake_minimum_required(VERSION 3.16)

project(FinanceTracker VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/database.cpp
    src/transaction.cpp
    src/category.cpp
    src/addtransactionpage.cpp
    src/transactionlistpage.cpp
    src/monthchartpage.cpp
    src/yearchartpage.cpp
    src/dashboardpage.cpp
    src/categorycombobox.cpp
    src/undonotification.cpp
    src/piechart.cpp
    src/barchart.cpp
    src/categorymanagerdialog.cpp
    # Investment models
    src/investmentcategory.cpp
    src/country.cpp
    src/currency.cpp
    src/snapshotposition.cpp
    src/snapshot.cpp
    src/withdrawal.cpp
    # Portfolio models
    src/portfolioasset.cpp
    src/assetoperation.cpp
    # Investment pages
    src/withdrawalspage.cpp
    src/withdrawalundonotification.cpp
    # Portfolio pages/dialogs
    src/portfoliopage.cpp
    src/addassetdialog.cpp
    src/assetdetailsdialog.cpp
    src/addoperationdialog.cpp
    src/createsnapshotdialog.cpp
    src/currenciespage.cpp
    src/currencycard.cpp
    src/assetcard.cpp
    src/snapshotshistorypage.cpp
    src/snapshotitemwidget.cpp
)

set(HEADERS
    include/mainwindow.h
    include/database.h
    include/transaction.h
    include/category.h
    include/addtransactionpage.h
    include/transactionlistpage.h
    include/monthchartpage.h
    include/yearchartpage.h
    include/dashboardpage.h
    include/categorycombobox.h
    include/undonotification.h
    include/piechart.h
    include/barchart.h
    include/categorymanagerdialog.h
    # Investment models
    include/investmentcategory.h
    include/country.h
    include/currency.h
    include/snapshotposition.h
    include/snapshot.h
    include/withdrawal.h
    # Portfolio models
    include/portfolioasset.h
    include/assetoperation.h
    # Investment pages
    include/withdrawalspage.h
    include/withdrawalundonotification.h
    # Portfolio pages/dialogs
    include/portfoliopage.h
    include/addassetdialog.h
    include/assetdetailsdialog.h
    include/addoperationdialog.h
    include/createsnapshotdialog.h
    include/currenciespage.h
    include/currencycard.h
    include/assetcard.h
    include/snapshotshistorypage.h
    include/snapshotitemwidget.h
)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    resources/app.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/style.qss
    ${CMAKE_CURRENT_BINARY_DIR}/style.qss
    COPYONLY
)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE app.rc)
endif()
