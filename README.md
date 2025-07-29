# DataTableModule for Unreal Engine

## 1. 개요

`DataTableModule`은 언리얼 엔진 에디터용 유틸리티 모듈로, Excel (`.xlsx`) 파일을 데이터 소스로 활용하는 과정을 자동화하고 간소화하기 위해 설계되었습니다.

게임 개발 시 기획자들이 Excel로 작성한 데이터를 엔진의 `UDataTable` 에셋으로 변환하는 작업은 반복적이고 실수가 잦을 수 있습니다. 이 모듈은 Excel 시트를 분석하여 CSV 파일과 그에 맞는 `USTRUCT` C++ 코드를 자동으로 생성하고, 최종적으로 데이터 테이블 에셋을 임포트하는 기능까지 통합된 UI를 통해 제공합니다.

## 2. 주요 기능

*   **Excel to CSV 변환**: `OpenXLSX` 라이브러리를 활용하여 Excel 파일의 특정 시트 또는 모든 시트를 CSV 파일로 변환합니다.
*   **UStruct 코드 자동 생성**: Excel 시트의 헤더(첫 번째 행)를 분석하여 `UDataTable`에서 요구하는 C++ `USTRUCT` 정의를 담은 헤더 파일(`.h`)을 자동으로 생성합니다.
*   **통합 관리 UI**: 에디터 내에서 위젯(`SDataTableManager`)을 통해 아래의 모든 작업을 직관적으로 관리할 수 있습니다.
    *   Excel, CSV, Struct 파일이 저장될 폴더 경로 지정
    *   지정된 폴더의 Excel 파일 목록 및 각 파일의 시트 목록 확인
    *   각 시트별 CSV 및 Struct 생성 여부 확인
    *   선택한 시트에 대한 CSV 변환, Struct 생성, DataTable 임포트 작업을 버튼 클릭으로 실행

## 3. 핵심 컴포넌트

### 3.1. `SDataTableManager` (Widget)

*   모듈의 메인 UI를 담당하는 Slate 위젯입니다.
*   사용자가 폴더 경로를 설정하고, Excel 파일과 시트를 선택하며, 변환/생성/임포트 명령을 내리는 인터페이스를 제공합니다.
*   `UDataTableManagerConfig`를 통해 사용자가 설정한 경로 등의 값을 저장하고 불러옵니다.

### 3.2. `XlsxManager` (Utility)

*   Excel 파일(`.xlsx`)을 직접 다루는 클래스입니다.
*   `OpenXLSX` 라이브러리를 통해 Excel 파일을 읽고, 시트 정보를 가져오며, 내용을 CSV 형식으로 변환하는 핵심 로직을 수행합니다.

### 3.3. `StructGenerator` (Utility)

*   CSV로 변환될 데이터의 구조에 맞춰 C++ `USTRUCT` 코드를 생성하는 클래스입니다.
*   Excel 시트의 첫 번째 행을 변수 타입으로, 두 번째 행을 변수명으로 인식하여 `FTableRowBase`를 상속받는 구조체 코드가 담긴 헤더 파일을 생성합니다.

### 3.4. `DataTableAssetGenerator` (Utility)

*   CSV를 임포트하여 데이터 테이블을 자동 생성하는 클래스입니다.

## 4. 의존성

*   **OpenXLSX**: C++ 환경에서 Excel 파일을 읽고 쓰기 위한 외부 라이브러리입니다. `DataTableModule.build.cs`에 해당 라이브러리의 헤더와 라이브러리 파일(.lib)이 포함되도록 설정되어 있습니다.

## 5. 사용법

1.  본 모듈을 프로젝트의 `Source` 폴더에 추가합니다.
2.  프로젝트의 `.uproject` 파일 또는 플러그인의 `.uplugin` 파일에 모듈을 추가하여 활성화합니다.
3.  언리얼 에디터를 실행한 후, 상단 메뉴의 'Tools' 또는 'Window'에서 `DataTableManager`를 찾아 실행합니다.
4.  UI 창에서 Excel 파일이 위치한 폴더와 CSV, Struct 파일을 저장할 폴더 경로를 지정합니다.
5.  목록에 나타난 Excel 시트 중 원하는 항목을 체크하고 아래의 버튼을 눌러 작업을 수행합니다.
    *   **Convert to CSV**: 선택된 시트를 CSV 파일로 변환합니다.
    *   **Generate Struct**: 선택된 시트의 구조에 맞는 UStruct 헤더 파일을 생성합니다.
    *   **Import DataTable**: 생성된 CSV와 UStruct를 기반으로 `UDataTable` 에셋을 지정된 경로에 생성합니다.

