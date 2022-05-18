using ClosedXML.Excel;

namespace use_closedxml
{
    class Program
    {
        static void Main(string[] args)
        {
            using (var book = new XLWorkbook())
            {
                var sheet = book.Worksheets.Add("Sample Sheet");
                sheet.Cell("A1").Value = "Hello";
                book.SaveAs("Hello.xlsx");
            }
        }
    }
}
