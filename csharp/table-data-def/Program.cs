using ClosedXML.Excel;
using System;

namespace table_data_def
{
    class Program
    {
        static void Main(string[] args)
        {
            using (var book = new XLWorkbook("sample.xlsx"))
            {
                foreach (var sheet in book.Worksheets)
                {
                    Console.Out.WriteLine(sheet.Name);
                    var range = sheet.RangeUsed();
                    foreach (var row in range.RowsUsed())
                    {
                        foreach (var cell in row.CellsUsed())
                        {
                            var address = cell.Address;
                            Console.Out.Write($"{address.RowNumber}-{address.ColumnNumber}={cell.Value}, ");
                        }
                        Console.Out.WriteLine();
                    }
                }
            }
        }
    }
}
